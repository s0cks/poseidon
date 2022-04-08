#include <deque>
#include <glog/logging.h>

#include "poseidon/utils.h"
#include "poseidon/local.h"
#include "poseidon/runtime.h"
#include "poseidon/scavenger.h"
#include "poseidon/allocator.h"
#include "poseidon/task_pool.h"

namespace poseidon{
 RelaxedAtomic<bool> Scavenger::scavenging_ = RelaxedAtomic<bool>(false);

 uword Scavenger::PromoteObject(RawObject* obj){
   GCLOG(1) << "promoting " << obj->ToString() << " to new zone.";
   auto new_ptr = (RawObject*) promotion_->Allocate(obj->GetPointerSize());
   CopyObject(obj, new_ptr);
   new_ptr->SetOldBit();

   stats_.num_promoted_ += 1;
   stats_.bytes_promoted_ += obj->GetPointerSize();
   return new_ptr->GetAddress();
 }

 uword Scavenger::ScavengeObject(RawObject* obj){
   GCLOG(1) << "scavenging " << obj->ToString() << " in old zone.";
   auto new_ptr = (RawObject*) zone()->Allocate(obj->GetPointerSize());
   CopyObject(obj, new_ptr);
   new_ptr->SetNewBit();

   stats_.num_scavenged_ += 1;
   stats_.bytes_scavenged_ += obj->GetPointerSize();
   return new_ptr->GetAddress();
 }

 uword Scavenger::ProcessObject(RawObject* raw){
   if(!raw->IsForwarding()){
     if(raw->IsRemembered()){
       auto new_address = PromoteObject(raw);
       ForwardObject(raw, new_address);
     } else{
       auto new_address = ScavengeObject(raw);
       ForwardObject(raw, new_address);
     }
   }
   PSDN_ASSERT(raw->IsForwarding());
   raw->SetRememberedBit();
   return raw->GetForwardingAddress();
 }

 void Scavenger::FinalizeObject(RawObject* obj){
   GCLOG(1) << "finalizing " << obj->ToString() << ".";
   stats_.num_finalized_ += 1;
   stats_.bytes_finalized_ += obj->GetPointerSize();
 }

 template<bool Parallel>
 class ScavengerVisitorBase : public RawObjectPointerVisitor{
  protected:
   Scavenger* scavenger_;

   explicit ScavengerVisitorBase(Scavenger* scavenger):
       RawObjectPointerVisitor(),
       scavenger_(scavenger){
   }

   inline Scavenger* scavenger() const{
     return scavenger_;
   }

   inline void SwapSpaces(){
     GCLOG(3) << "swapping spaces.";
     return scavenger()->SwapSpaces();
   }

#ifdef PSDN_DEBUG
   inline void ClearToSpace(){
     Semispace tospace(scavenger()->zone()->tospace(), scavenger()->zone()->semisize());
     GCLOG(3) << "clearing tospace: " << tospace;
     tospace.Clear();
   }
#endif//PSDN_DEBUG

   virtual void ProcessRoots() = 0;
   virtual void ProcessToSpace() = 0;
  public:
   ~ScavengerVisitorBase() override = default;

   bool IsParallel() const{
     return Parallel;
   }
 };

 class SerialScavengerVisitor : public ScavengerVisitorBase<false>{
  private:
   std::deque<uword> work_;

   inline bool
   HasWork() const{
     return !work_.empty();
   }

   void ProcessRoots() override{
     auto locals = LocalPage::GetLocalPageForCurrentThread();
     locals->VisitPointers([&](RawObject** ptr){
       auto old_val = (*ptr);
       if(old_val->IsNew() && !old_val->IsForwarding()){
         auto new_val = (RawObject*) scavenger()->ProcessObject(old_val);
         new_val->SetRememberedBit();
         (*ptr) = new_val;
       }
       return true;
     });
   }

   void ProcessToSpace() override{

   }
  public:
   explicit SerialScavengerVisitor(Scavenger* scavenger):
     ScavengerVisitorBase<false>(scavenger){
   }
   ~SerialScavengerVisitor() override = default;

   bool Visit(RawObject** ptr) override{
     return true;
   }

   void ScavengeMemory(){
     SwapSpaces();
     ProcessRoots();
     ProcessToSpace();

#ifdef PSDN_DEBUG
     ClearToSpace();
#endif//PSDN_DEBUG
   }
 };

 class ParallelScavengeTask : public Task{
   friend class ParallelScavengerVisitor;
  private:
   Scavenger* scavenger_;
   WorkStealingQueue<uword>* work_;

   ParallelScavengeTask(Scavenger* scavenger, WorkStealingQueue<uword>* work):
     scavenger_(scavenger),
     work_(work){
   }

   inline Scavenger* scavenger() const{
     return scavenger_;
   }
  public:
   ~ParallelScavengeTask() override = default;

   const char* name() const override{
     return "ParallelScavengerTask";
   }

   bool HasWork() const{
     return !work_->empty();
   }

   void Run() override{
     do{
       do{
         uword next;
         if((next = work_->Steal()) != 0){
           auto old_val = (RawObject*)next;
           auto new_val = (RawObject*)scavenger()->ProcessObject(old_val);
           new_val->SetRememberedBit();
         }
       } while(HasWork());
     } while(Scavenger::IsScavenging());
   }
 };

 class ParallelScavengerVisitor : public ScavengerVisitorBase<true>{
  protected:
   WorkStealingQueue<uword>* work_;

   void ProcessRoots() override{
     GCLOG(3) << "processing roots.....";
     auto locals = LocalPage::GetLocalPageForCurrentThread();
     locals->VisitPointers([&](RawObject** ptr){
       auto old_val = (*ptr);
       if(old_val->IsNew() && !old_val->IsForwarding()){
         GCLOG(10) << "pushing " << old_val->ToString();
         scavenger()->work().Push(old_val->GetAddress());
       }
       return true;
     });
   }

   void ProcessToSpace() override{
     GCLOG(3) << "processing to-space....";
     auto locals = LocalPage::GetLocalPageForCurrentThread();
     locals->VisitPointers([&](RawObject** val){
       auto old_val = (*val);
       if(old_val && old_val->IsForwarding()){
         auto new_val = (RawObject*)old_val->GetForwardingAddress();
         GCLOG(3) << "forwarding " << (val) << " to " << new_val->ToString();
         (*val) = (RawObject*)old_val->GetForwardingAddress();
       }
       return true;
     });
   }
  public:
   explicit ParallelScavengerVisitor(Scavenger* scavenger):
     ScavengerVisitorBase<true>(scavenger){
     for(auto idx = 0; idx < TaskPool::kDefaultNumberOfWorkers; idx++){
       Runtime::GetTaskPool()->Submit(new ParallelScavengeTask(scavenger, &scavenger->work()));
     }
   }
   ~ParallelScavengerVisitor() override = default;

   bool Visit(RawObject** ptr) override{
     auto old_val = (*ptr);
     if(old_val != nullptr)
       scavenger()->work().Push((uword) ptr);
     return true;
   }

   void ScavengeMemory(){
     SwapSpaces();

     ProcessRoots();
     ProcessToSpace();

#ifdef PSDN_DEBUG
     ClearToSpace();
#endif//PSDN_DEBUG
   }
 };

 void Scavenger::SerialScavenge(){
   return ScavengeMemory<SerialScavengerVisitor>();
 }

 void Scavenger::ParallelScavenge(){
   return ScavengeMemory<ParallelScavengerVisitor>();
 }

 void Scavenger::Scavenge(){
   if(ShouldUseParallelScavenger()){
     GCLOG(1) << "using parallel scavenger.";
     ParallelScavenge();
   } else{
     GCLOG(1) << "using serial scavenger.";
     SerialScavenge();
   }
 }
}