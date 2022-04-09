#include <deque>
#include <glog/logging.h>

#include "poseidon/utils.h"
#include "poseidon/local.h"
#include "poseidon/runtime.h"
#include "poseidon/scavenger.h"
#include "poseidon/allocator.h"
#include "poseidon/task_pool.h"

namespace poseidon{
 static RelaxedAtomic<bool> scavenging(false);

 bool Scavenger::IsScavenging(){
   return (bool)scavenging;
 }

 void Scavenger::SetScavenging(){
   scavenging = true;
 }

 void Scavenger::ClearScavenging(){
   scavenging = false;
 }

 template<bool Parallel>
 class ScavengerVisitorBase : public RawObjectPointerVisitor{
  protected:
   NewZone* zone_;
   Semispace from_;
   Semispace to_;
   OldZone* promotion_;

   explicit ScavengerVisitorBase(Heap* heap):
    RawObjectPointerVisitor(),
    zone_(heap->new_zone()),
    from_(heap->new_zone()->fromspace(), heap->new_zone()->semisize()),
    to_(heap->new_zone()->tospace(), heap->new_zone()->semisize()),
    promotion_(heap->old_zone()){
   }

   inline void SwapSpaces(){
     GCLOG(3) << "swapping spaces.";
     return zone_->SwapSpaces();
   }

   inline void ClearFromSpace(){
     from_.Clear();
   }

   static inline void
   ForwardObject(RawObject* obj, uword forwarding_address){
     GCLOG(1) << "forwarding " << obj->ToString() << " to " << ((RawObject*) forwarding_address)->ToString();
     obj->SetForwardingAddress(forwarding_address);
     PSDN_ASSERT(obj->GetForwardingAddress() == forwarding_address);
   }

   static inline void
   CopyObject(RawObject* src, RawObject* dst){//TODO: create a better copy
     //   PSDN_ASSERT(src->GetTotalSize() == dst->GetTotalSize());
     memcpy(dst->GetPointer(), src->GetPointer(), src->GetPointerSize());
   }

   inline uword PromoteObject(RawObject* raw){
     GCLOG(1) << "promoting " << raw->ToString() << " to new zone.";
     auto new_ptr = (RawObject*)promotion_->Allocate(raw->GetPointerSize());
     CopyObject(raw, new_ptr);
     new_ptr->SetOldBit();
//TODO:
//   stats_.num_promoted_ += 1;
//   stats_.bytes_promoted_ += obj->GetPointerSize();
     return new_ptr->GetAddress();
   }

   inline uword ScavengeObject(RawObject* raw){
     GCLOG(1) << "scavenging " << raw->ToString() << " in old zone.";
     auto new_ptr = (RawObject*)to_.Allocate(raw->GetPointerSize());
     CopyObject(raw, new_ptr);
     new_ptr->SetNewBit();

//TODO:
//   stats_.num_scavenged_ += 1;
//   stats_.bytes_scavenged_ += obj->GetPointerSize();
     return new_ptr->GetAddress();
   }

   inline uword ProcessObject(RawObject* raw){
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

   inline void FinalizeObject(RawObject* raw){
     GCLOG(1) << "finalizing " << raw->ToString() << ".";
//TODO:
//   stats_.num_finalized_ += 1;
//   stats_.bytes_finalized_ += obj->GetPointerSize();
   }

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
         auto new_val = (RawObject*)ProcessObject(old_val);
         new_val->SetRememberedBit();
         (*ptr) = new_val;
       }
       return true;
     });
   }

   void ProcessToSpace() override{

   }
  public:
   explicit SerialScavengerVisitor(Heap* heap):
     ScavengerVisitorBase<false>(heap){
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
     ClearFromSpace();
#endif//PSDN_DEBUG
   }
 };

 class ParallelScavengerVisitor;
 class ParallelScavengerTask : public Task{
   friend class ParallelScavengerVisitor;
  private:
   ParallelScavengerVisitor* scavenger_;

   inline ParallelScavengerVisitor* scavenger(){
     return scavenger_;
   }

   uword GetNext();
   uword ProcessObject(RawObject* raw);
  public:
   explicit ParallelScavengerTask(ParallelScavengerVisitor* scavenger):
    Task(),
    scavenger_(scavenger){
   }
   ~ParallelScavengerTask() override = default;

   const char* name() const override{
     return "ParallelScavengerTask";
   }

   bool HasWork();

   void Run() override{
     do{
       do{
         uword next;
         if((next = GetNext()) != 0){
           auto old_val = (RawObject*)next;
           auto new_val = (RawObject*)ProcessObject(old_val);
           new_val->SetRememberedBit();
         }
       } while(HasWork());
     } while(Scavenger::IsScavenging());
   }
 };

 class ParallelScavengerVisitor : public ScavengerVisitorBase<true>{
   friend class ParallelScavengerTask;
  protected:
   WorkStealingQueue<uword> work_;

   inline WorkStealingQueue<uword>* work(){
     return &work_;
   }

   void ProcessRoots() override{
     GCLOG(3) << "processing roots.....";
     auto locals = LocalPage::GetLocalPageForCurrentThread();
     locals->VisitPointers([&](RawObject** ptr){
       auto old_val = (*ptr);
       if(old_val->IsNew() && !old_val->IsForwarding()){
         GCLOG(10) << "pushing " << old_val->ToString();
         work_.Push(old_val->GetAddress());
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
   explicit ParallelScavengerVisitor(Heap* heap):
     ScavengerVisitorBase<true>(heap),
     work_(4096){
   }
   ~ParallelScavengerVisitor() override = default;

   bool Visit(RawObject** ptr) override{
     auto old_val = (*ptr);
     if(old_val != nullptr)
       work_.Push((uword) ptr);
     return true;
   }

   void ScavengeMemory(){
     SwapSpaces();

     ProcessRoots();
     ProcessToSpace();

#ifdef PSDN_DEBUG
     ClearFromSpace();
#endif//PSDN_DEBUG
   }
 };

 bool ParallelScavengerTask::HasWork(){
   return !scavenger()->work()->empty();
 }

 uword ParallelScavengerTask::GetNext(){
   return scavenger()->work()->Steal();
 }

 uword ParallelScavengerTask::ProcessObject(RawObject* raw){
   return scavenger()->ProcessObject(raw);
 }

 void Scavenger::SerialScavenge(){
   auto heap = Heap::GetCurrentThreadHeap();
   SerialScavengerVisitor visitor(heap);

   DTIMED_SECTION("SerialScavenge", {
     visitor.ScavengeMemory();
   });
 }

 void Scavenger::ParallelScavenge(){
   auto heap = Heap::GetCurrentThreadHeap();
   ParallelScavengerVisitor visitor(heap);

   //TODO: cleanup this loop
   for(auto idx = 0; idx < GetNumberOfWorkers(); idx++)
     Runtime::GetTaskPool()->Submit(new ParallelScavengerTask(&visitor));

   DTIMED_SECTION("ParallelScavenge", {
     visitor.ScavengeMemory();
   });
 }

 void Scavenger::ScavengeMemory(){
   if(IsScavenging()){
     DLOG(WARNING) << "already scavenging.";
     return;
   }

   SetScavenging();
   if(ShouldUseParallelScavenge()){
     ParallelScavenge();
   } else{
     SerialScavenge();
   }
   ClearScavenging();
 }
}