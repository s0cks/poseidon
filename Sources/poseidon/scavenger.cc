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
     GCLOG(10) << "forwarding " << obj->ToString() << " to " << ((RawObject*) forwarding_address)->ToString();
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
     auto new_ptr = (RawObject*)promotion_->TryAllocate(raw->GetPointerSize());
     CopyObject(raw, new_ptr);
     new_ptr->SetOldBit();
//TODO:
//   stats_.num_promoted_ += 1;
//   stats_.bytes_promoted_ += obj->GetPointerSize();
     return new_ptr->GetAddress();
   }

   inline uword ScavengeObject(RawObject* raw){
     GCLOG(1) << "scavenging " << raw->ToString() << " in old zone.";
     auto new_ptr = (RawObject*)to_.TryAllocate(raw->GetPointerSize());
     new_ptr->SetPointerSize(raw->GetPointerSize());
     new_ptr->SetNewBit();

     CopyObject(raw, new_ptr);

//TODO:
//   stats_.num_scavenged_ += 1;
//   stats_.bytes_scavenged_ += obj->GetPointerSize();
     return new_ptr->GetAddress();
   }

   inline uword ProcessObject(RawObject* raw){
     DLOG(INFO) << "processing " << raw->ToString();
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

 class SerialScavenger : public ScavengerVisitorBase<false>{
  private:
   void ProcessLocals(){
     TIMED_SECTION("ProcessLocals", {
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
     });
   }

   void ProcessRoots() override{
     TIMED_SECTION("ProcessRoots", {
       ProcessLocals();
       //TODO: process old to new references
     });
   }

   void ProcessToSpace() override{
     TIMED_SECTION("ProcessToSpace", {
       zone_->VisitObjectPointers([&](RawObject* val){
         if(val->IsForwarding()){
           DLOG(INFO) << "scavenged " << val->ToString();
         } else{
           DLOG(INFO) << "freeing " << val->ToString();
         }
         return true;
       });
     });
   }

   inline void ProcessAll(){
     ProcessRoots();
     ProcessToSpace();
   }

   void NotifyLocals(){
     DTIMED_SECTION("NotifyLocals", {
       auto locals = LocalPage::GetLocalPageForCurrentThread();
       locals->VisitPointers([&](RawObject** ptr){
         auto old_val = (*ptr);
         if(old_val->IsNew() && old_val->IsForwarding()){
           (*ptr) = (RawObject*)old_val->GetForwardingAddress();
         }
         return true;
       });
     });
   }
  public:
   explicit SerialScavenger(Heap* heap):
     ScavengerVisitorBase<false>(heap){
   }
   ~SerialScavenger() override = default;

   bool Visit(RawObject** ptr) override{
     return true;
   }

   void ScavengeMemory(){
     SwapSpaces();

     ProcessAll();
     NotifyLocals();

#ifdef PSDN_DEBUG
     ClearFromSpace();
#endif//PSDN_DEBUG
   }
 };

 class ParallelScavenger;
 class ParallelScavengerTask : public Task{
   friend class ParallelScavenger;
  private:
   ParallelScavenger* scavenger_;

   inline ParallelScavenger* scavenger(){
     return scavenger_;
   }

   uword GetNext();
   uword ProcessObject(RawObject* raw);
  public:
   explicit ParallelScavengerTask(ParallelScavenger* scavenger):
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

 class ParallelScavenger : public ScavengerVisitorBase<true>{
   friend class ParallelScavengerTask;
  protected:
   WorkStealingQueue<uword> work_;

   inline WorkStealingQueue<uword>* work(){
     return &work_;
   }

   void ProcessLocals(){
     TIMED_SECTION("ProcessLocals", {
       auto locals = LocalPage::GetLocalPageForCurrentThread();
       locals->VisitPointers([&](RawObject** ptr){
         auto old_val = (*ptr);
         if(old_val->IsNew() && !old_val->IsForwarding()){
           work_.Push(old_val->GetAddress());
         }
         return true;
       });

       while(!work_.empty());//spin
     });
   }

   void ProcessRoots() override{
     TIMED_SECTION("ProcessRoots", {
       ProcessLocals();
       //TODO: process old to new references
     });
   }

   void ProcessToSpace() override{
     TIMED_SECTION("ProcessToSpace", {
       auto address = to_.GetStartingAddress();
       while(address < to_.GetEndingAddress() && ((RawObject*)address)->GetPointerSize() > 0){
         auto ptr = (RawObject*)address;
         if(ptr->IsForwarding()){
           DLOG(INFO) << "processing " << ptr->ToString();
         }
         address += ptr->GetTotalSize();
       }
     });
   }

   inline void ProcessAll(){
     ProcessRoots();
     ProcessToSpace();
   }

   void NotifyLocals(){
     DTIMED_SECTION("NotifyLocals", {
       auto locals = LocalPage::GetLocalPageForCurrentThread();
       locals->VisitPointers([&](RawObject** ptr){
         auto old_val = (*ptr);
         if(old_val->IsNew() && old_val->IsForwarding()){
           (*ptr) = (RawObject*)old_val->GetForwardingAddress();
         }
         return true;
       });
     });
   }
  public:
   explicit ParallelScavenger(Heap* heap):
     ScavengerVisitorBase<true>(heap),
     work_(4096){
   }
   ~ParallelScavenger() override = default;

   bool Visit(RawObject** ptr) override{
     auto old_val = (*ptr);
     if(old_val != nullptr)
       work_.Push((uword) ptr);
     return true;
   }

   void ScavengeMemory(){
     SwapSpaces();

     ProcessAll();
     NotifyLocals();

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
   SerialScavenger visitor(heap);

   TIMED_SECTION("SerialScavenge", {
     visitor.ScavengeMemory();
   });
 }

 void Scavenger::ParallelScavenge(){
   auto heap = Heap::GetCurrentThreadHeap();
   ParallelScavenger visitor(heap);
   Runtime::GetTaskPool()->SubmitToAll<ParallelScavengerTask>(&visitor);
   TIMED_SECTION("ParallelScavenge", {
     visitor.ScavengeMemory();
   });
 }

 void Scavenger::Scavenge(){
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