#include <deque>
#include <glog/logging.h>

#include "poseidon/utils.h"
#include "poseidon/local.h"
#include "poseidon/runtime.h"
#include "poseidon/task_pool.h"

#include "poseidon/collector/scavenger.h"
#include "poseidon/collector/finalizer.h"
#include "poseidon/heap/new_zone.h"
#include "poseidon/heap/semispace.h"

namespace poseidon{
 static RelaxedAtomic<bool> scavenging(false);
 static RelaxedAtomic<int64_t> processing(0);

 static AtomicTimestamp last_scavenge_ts;
 static AtomicLong last_scavenge_duration_ms;
 static AtomicPointerCounter last_scavenge_scavenged_;
 static AtomicPointerCounter last_scavenge_promoted_;

 bool Scavenger::IsScavenging(){
   return (bool)scavenging;
 }

 void Scavenger::SetScavenging(bool active){
   if(active){
     last_scavenge_ts = Clock::now();
     scavenging = true;
     last_scavenge_promoted_ = 0;
     last_scavenge_scavenged_ = 0;
   } else{
     scavenging = false;
     last_scavenge_duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - GetLastScavengeTimestamp()).count();
   }
 }

 ScavengerStats Scavenger::GetStats(){
   return { last_scavenge_ts,
           last_scavenge_duration_ms,
           last_scavenge_scavenged_,
           last_scavenge_promoted_,
           Finalizer::finalized() };
 }

 Timestamp Scavenger::GetLastScavengeTimestamp(){
   return (Timestamp)last_scavenge_ts;
 }

 int64_t Scavenger::GetLastScavengeDurationMillis(){
   return (int64_t)last_scavenge_duration_ms;
 }

 int64_t Scavenger::GetNumberOfObjectsScavengedLastScavenge(){
   return (int64_t)last_scavenge_scavenged_.count;
 }

 int64_t Scavenger::GetNumberOfBytesScavengedLastScavenge(){
   return (int64_t)last_scavenge_scavenged_.bytes;
 }

 int64_t Scavenger::GetNumberOfObjectsPromotedLastScavenge(){
   return (int64_t)last_scavenge_promoted_.count;
 }

 int64_t Scavenger::GetNumberOfBytesPromotedLastScavenge(){
   return (int64_t)last_scavenge_promoted_.bytes;
 }

 uword Scavenger::ScavengeObject(Semispace* tospace, RawObject* ptr){
   DLOG(INFO) << "scavenging " << (*ptr);
   auto new_ptr = (RawObject*)tospace->TryAllocate(ptr->GetPointerSize());
   new_ptr->SetPointerSize(ptr->GetPointerSize());
   CopyObject(ptr, new_ptr);
   last_scavenge_scavenged_ += ptr;
   return new_ptr->GetStartingAddress();
 }

 uword Scavenger::PromoteObject(OldZone* zone, RawObject* ptr){
   DLOG(INFO) << "promoting " << (*ptr) << " to new zone.";
   auto new_ptr = (RawObject*)zone->TryAllocate(ptr->GetPointerSize());
   CopyObject(ptr, new_ptr);
   last_scavenge_promoted_ += ptr;
   return new_ptr->GetStartingAddress();
 }

 uword Scavenger::ProcessObject(Semispace* tospace, OldZone* old_zone, RawObject* ptr){
   DLOG(INFO) << "processing " << (*ptr);
   if(!ptr->IsForwarding()){
     if(ptr->IsRemembered()){
       auto new_address = PromoteObject(old_zone, ptr);
       ForwardObject(ptr, new_address);
     } else{
       auto new_address = ScavengeObject(tospace, ptr);
       ForwardObject(ptr, new_address);
     }
   }

   PSDN_ASSERT(ptr->IsForwarding());
   auto new_ptr = (RawObject*)ptr->GetForwardingAddress();
   new_ptr->SetRememberedBit();
   return new_ptr->GetStartingAddress();
 }

 void Scavenger::ForwardObject(RawObject* obj, uword forwarding_address){
   DLOG(INFO) << "forwarding " << (*obj) << " to " << (*((RawObject*) forwarding_address));
   obj->SetForwardingAddress(forwarding_address);
   PSDN_ASSERT(obj->GetForwardingAddress() == forwarding_address);
 }

 void Scavenger::CopyObject(RawObject* src, RawObject* dst){//TODO: create a better copy
   //   PSDN_ASSERT(src->GetTotalSize() == dst->GetTotalSize());
   memcpy(dst->GetPointer(), src->GetPointer(), src->GetPointerSize());
 }

 template<bool Parallel>
 class ScavengerVisitorBase : public RawObjectPointerVisitor{
  protected:
   NewZone* zone_;
   Semispace& from_;
   Semispace& to_;
   OldZone* promotion_;

   explicit ScavengerVisitorBase(Heap* heap):
    RawObjectPointerVisitor(),
    zone_(&heap->new_zone()),
    from_(heap->new_zone().fromspace()),
    to_(heap->new_zone().tospace()),
    promotion_(&heap->old_zone()){
   }

   inline void SwapSpaces(){
     GCLOG(3) << "swapping spaces.";
     return zone_->SwapSpaces();
   }

   inline void ClearFromSpace(){
     Scavenger::Clear(&from_);
   }

   inline void FinalizeObject(RawObject* raw){
     GCLOG(1) << "finalizing " << (*raw) << ".";
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
           auto new_val = (RawObject*)Scavenger::ProcessObject(&to_, promotion_, old_val);
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
       NOT_IMPLEMENTED(ERROR); //TODO: implement
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

     ClearFromSpace();//TODO: Change to finalizer
   }
 };

 class ParallelScavenger;
 class ParallelScavengerTask : public Task{
   friend class ParallelScavenger;
  private:
   ParallelScavenger* scavenger_;
   WorkStealingQueue<uword>* work_;

   inline WorkStealingQueue<uword>* work() const{
     return work_;
   }

   uword GetNext();
   uword ProcessObject(RawObject* raw);
  public:
   explicit ParallelScavengerTask(ParallelScavenger* scavenger);
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
           processing += 1;
           auto old_val = (RawObject*)next;
           auto new_val = (RawObject*)ProcessObject(old_val);
           new_val->SetRememberedBit();
           processing -= 1;
         }
       } while(HasWork());
     } while(Scavenger::IsScavenging());
   }
 };

 class ParallelScavenger : public ScavengerVisitorBase<true>{
   friend class ParallelScavengerTask;
  protected:
   WorkStealingQueue<uword>* work_;

   inline WorkStealingQueue<uword>* work() const{
     return work_;
   }

   inline bool HasWork() const{
     return !work_->empty() || processing > 0;
   }

   void ProcessLocals(){
     DTIMED_SECTION("ProcessLocals", {
       auto locals = LocalPage::GetLocalPageForCurrentThread();
       locals->VisitPointers([&](RawObject** ptr){
         auto old_val = (*ptr);
         if(old_val->IsNew() && !old_val->IsForwarding())
           work_->Push(old_val->GetStartingAddress());
         return true;
       });
     });
   }

   void ProcessRoots() override{
     DTIMED_SECTION("ProcessRoots", {
       ProcessLocals();
       //TODO: process old to new references

       // wait for work to finish.
       while(HasWork());//spin
     });
   }

   void ProcessToSpace() override{
     DTIMED_SECTION("ProcessToSpace", {
       auto address = to_.GetStartingAddress();
       while(address < to_.GetEndingAddress() && ((RawObject*)address)->IsNew()){
         auto ptr = (RawObject*)address;
         if(ptr->IsRemembered()){
           DLOG(INFO) << "processing " << (*ptr);
           //TODO: process references from ptr
         }
         address += ptr->GetTotalSize();
       }

       // wait for work to finish.
       while(HasWork());//spin
     });
   }

   inline void ProcessAll(){
     ProcessRoots();
     ProcessToSpace();
   }

   void NotifyLocals(){//TODO: serialize
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
     work_(new WorkStealingQueue<uword>(1024)){
   }
   ~ParallelScavenger() override{
     delete work_;
   }

   bool Visit(RawObject** ptr) override{
     auto old_val = (*ptr);
     if(old_val != nullptr)
       work_->Push((uword) ptr);
     return true;
   }

   void ScavengeMemory(){
     SwapSpaces();

     ProcessAll();
     NotifyLocals();

     ClearFromSpace();
   }
 };

 ParallelScavengerTask::ParallelScavengerTask(ParallelScavenger* scavenger):
   Task(),
   work_(scavenger->work()),
   scavenger_(scavenger){
 }

 bool ParallelScavengerTask::HasWork(){
   return !work()->empty();
 }

 uword ParallelScavengerTask::GetNext(){
   return work()->Steal();
 }

 uword ParallelScavengerTask::ProcessObject(RawObject* raw){//TODO: refactor ->zone_ & ->promotion_
   return Scavenger::ProcessObject(&scavenger_->to_, scavenger_->promotion_, raw);
 }

 void Scavenger::SerialScavenge(Heap* heap){
   SerialScavenger visitor(heap);
   TIMED_SECTION("SerialScavenge", {
     visitor.ScavengeMemory();
   });
 }

 void Scavenger::ParallelScavenge(Heap* heap){
   auto scavenger = new ParallelScavenger(heap);
   Runtime::GetTaskPool()->SubmitToAll<ParallelScavengerTask>(scavenger);
   DTIMED_SECTION("ParallelScavenge", {
     scavenger->ScavengeMemory();
   });
 }

 void Scavenger::Scavenge(Heap* heap, bool parallel){
   if(IsScavenging()){
     DLOG(WARNING) << "already scavenging.";
     return;
   }

   DLOG(INFO) << "scavenger stats (before): " << GetStats();
   SetScavenging();
   if(parallel){
     ParallelScavenge(heap);
   } else{
     SerialScavenge(heap);
   }
   ClearScavenging();
   DLOG(INFO) << "scavenger stats (after): " << GetStats();
 }
}