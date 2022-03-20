#include <deque>
#include <glog/logging.h>

#include "poseidon/utils.h"
#include "poseidon/local.h"
#include "poseidon/scavenger.h"
#include "poseidon/allocator.h"
#include "poseidon/task_pool.h"

namespace poseidon{
 uword Scavenger::PromoteObject(RawObject* obj){
   DVLOG(1) << "promoting " << obj->ToString() << " to new zone.";
   auto new_ptr = (RawObject*)Heap::GetCurrentThreadHeap()->old_zone()->Allocate(obj->GetPointerSize());
   CopyObject(obj, new_ptr);
   new_ptr->SetOldBit();

   num_promoted_ += 1;
   bytes_promoted_ += obj->GetPointerSize();//TODO: should this be total size (object size + header size) instead?
   return new_ptr->GetAddress();
 }

 uword Scavenger::ScavengeObject(RawObject* obj){
   DVLOG(1) << "scavenging " << obj->ToString() << " in old zone.";
   auto new_ptr = (RawObject*)zone()->Allocate(obj->GetPointerSize());
   CopyObject(obj, new_ptr);
   new_ptr->SetNewBit();

   num_scavenged_ += 1;
   bytes_scavenged_ += obj->GetPointerSize();//TODO: should this be total size (object size + header size) instead?
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
   raw->SetRememberedBit();
   return raw->GetForwardingAddress();
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
  public:
   ~ScavengerVisitorBase() override = default;

   bool IsParallel() const{
     return Parallel;
   }
 };

 class SerialScavengerVisitor : public ScavengerVisitorBase<false>{
  public:
   explicit SerialScavengerVisitor(Scavenger* scavenger):
     ScavengerVisitorBase<false>(scavenger){
   }
   ~SerialScavengerVisitor() override = default;

   bool Visit(RawObject** ptr) override{
     return true;
   }
 };

 class ParallelScavengeTask : public Task, public RawObjectPointerVisitor{
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

   inline RawObject** next_ptr(){
     return (RawObject**)work_->Steal();
   }
  public:
   ~ParallelScavengeTask() override = default;

   const char* name() const override{
     return "ParallelScavengerTask";
   }

   bool Visit(RawObject** ptr) override{
     auto old_val = (*ptr);
     if(old_val->IsForwarding()){

     }
     return true;
   }

   void Run() override{
     RawObject** ptr;
     while(!work_->empty()){
       if((ptr = next_ptr()) != nullptr){
         auto old_val = (*ptr);
         if(old_val->IsNew() && !old_val->IsRemembered() && !old_val->IsForwarding()){
           auto new_val = (RawObject*)scavenger()->ProcessObject(old_val);
           new_val->SetRememberedBit();
           (*ptr) = new_val;
         }
       }
     }
   }
 };


 class ParallelScavengerVisitor : public ScavengerVisitorBase<true>{
  protected:
   TaskPool pool_;
   WorkStealingQueue<uword> work_;
  public:
   explicit ParallelScavengerVisitor(Scavenger* scavenger):
    ScavengerVisitorBase<true>(scavenger),
    pool_(),
    work_(){
   }
   ~ParallelScavengerVisitor() override = default;

   bool Visit(RawObject** ptr) override{
     work_.Push((uword)ptr);
     return true;
   }

   void ScavengeMemory(){
     Allocator::VisitRoots(this);
     for(auto idx = 0; idx < TaskPool::kDefaultNumberOfWorkers; idx++)
       pool_.Submit(new ParallelScavengeTask(scavenger(), &work_));
     while(!work_.empty());// spin
   }
 };

 void Scavenger::Scavenge(){
#ifdef PSDN_MTA
   DLOG(INFO) << "performing ParallelScavenge.....";
   ParallelScavengerVisitor visitor(this);
#else
   DLOG(INFO) << "performing SerialScavenge.....";
   SerialScavengerVisitor visitor(this);
#endif//PSDN_MTA

#ifdef PSDN_DEBUG
   auto start_ts = Clock::now();
#endif//PSDN_DEBUG

   visitor.ScavengeMemory();

#ifdef PSDN_DEBUG
   auto finish_ts = Clock::now();
   DLOG(INFO) << "finished in " << (finish_ts - start_ts) << ".";
   DLOG(INFO) << "number of objects scavenged: " << num_scavenged_ << " (" << Bytes(bytes_scavenged_) << ").";
   DLOG(INFO) << "number of objects promoted: " << num_promoted_ << " (" << Bytes(bytes_promoted_) << ").";
#endif//PSDN_DEBUG
 }
}