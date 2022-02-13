#include <deque>
#include <glog/logging.h>

#include "utils.h"
#include "scavenger.h"
#include "allocator.h"

#include "finalizer.h"

namespace poseidon{
 class CollectionStats{
  private:
   Heap::HeapStats& start_;
   Timestamp& start_ts_;
   Heap::HeapStats& finished_;
   Timestamp& finished_ts_;
  public:
   CollectionStats(Heap::HeapStats& start, Timestamp& start_ts,
                   Heap::HeapStats& finished, Timestamp& finished_ts):
       start_(start),
       start_ts_(start_ts),
       finished_(finished),
       finished_ts_(finished_ts){
   }
   ~CollectionStats() = default;

   Heap::HeapStats& GetStartStats() const{
     return start_;
   }

   Timestamp& GetStartTimestamp() const{
     return start_ts_;
   }

   Heap::HeapStats& GetFinishedStats() const{
     return finished_;
   }

   Timestamp& GetFinishedTimestamp() const{
     return finished_ts_;
   }

   Duration GetDuration() const{
     return (GetFinishedTimestamp() - GetStartTimestamp());
   }

   uint64_t GetFreedBytes() const{
     auto allocated_before = GetStartStats().GetAllocatedBytes();
     auto allocated_after = GetFinishedStats().GetAllocatedBytes();
     return allocated_before - allocated_after;
   }

   uint64_t GetTotalBytes() const{
     return GetStartStats().GetTotalBytes();
   }

   double GetFreedPercentage() const{
     return GetPercentageOf(GetFreedBytes(), GetTotalBytes());
   }
 };

 uword Scavenger::CopyObject(RawObjectPtr raw){
   if(!raw->IsForwarding()){
     if(raw->IsNew() && !raw->IsMarked() && raw->IsRemembered()){
       auto new_ptr = Allocator::GetOldSpace()->AllocateRawObject(raw->GetPointerSize());
       raw->SetForwardingAddress((uword)new_ptr);
       raw->SetMarkedBit();

       memcpy(new_ptr->GetPointer(), raw->GetPointer(), raw->GetPointerSize());
       new_ptr->SetOldBit();
       new_ptr->GetObjectPointer()->set_raw(new_ptr);
       DLOG(INFO) << "promoted " << raw->ToString() << " to " << new_ptr->ToString();
     } else if(raw->IsNew() && !raw->IsMarked() && !raw->IsRemembered()){
       auto new_ptr = GetHeap()->AllocateRawObject(raw->GetPointerSize());
       raw->SetForwardingAddress((uword)new_ptr);
       raw->SetMarkedBit();

       memcpy(new_ptr->GetPointer(), raw->GetPointer(), raw->GetPointerSize());
       new_ptr->SetNewBit();
       new_ptr->SetRememberedBit();
       new_ptr->GetObjectPointer()->set_raw(new_ptr);
       DLOG(INFO) << "relocated " << raw->ToString() << " to " << new_ptr->ToString();
     }
   }
   return raw->GetForwardingAddress();
 }

 bool Scavenger::Visit(RawObjectPtr* ptr){
   auto old_val = (*ptr);
   if(!old_val->IsMarked() && !old_val->IsRemembered()){
     (*ptr) = (RawObject*)CopyObject(old_val);
   }
   return true;
 }

 bool Scavenger::Visit(RawObjectPtr ptr){
   DLOG(INFO) << "visiting pointers in " << ptr->ToString();
   ptr->VisitPointers(this);
   return true;
 }

 void Scavenger::ProcessRoots(){
   DLOG(INFO) << "processing roots....";
   Allocator::VisitLocals(this);
 }

 void Scavenger::ProcessToSpace(){
   DLOG(INFO) << "processing to-space " << Allocator::GetNewSpace()->GetFromSpace() << " (" << PrettyPrintPercentage(Allocator::GetNewSpace()->GetFromSpace().GetAllocatedPercentage()) << ")....";
   Allocator::GetNewSpace()->GetFromSpace().VisitRawObjectPointers(this);
 }

 void Scavenger::ProcessCopiedObjects(){
   NOT_IMPLEMENTED(ERROR);//TODO: implement
 }

 void Scavenger::MinorCollection(){
   // only collect from the eden heap
   DLOG(INFO) << "executing minor collection....";
   auto heap = Allocator::GetNewSpace();
   auto start_ts = Clock::now();
   auto num_allocated = Allocator::GetNumberOfObjectsAllocated();
   auto start_stats = heap->GetStats();

   Scavenger scavenger(Allocator::GetNewSpace());//allocate in from_
   heap->SwapSpaces(); // from_ = to_; to_ = from_
   scavenger.ProcessRoots(); // scavenge roots into to_
   scavenger.ProcessToSpace(); // scavenge from_ into to_
   heap->GetToSpace().Clear();

   auto finished_ts = Clock::now();
   auto finished_stats = heap->GetStats();
   CollectionStats stats(start_stats, start_ts, finished_stats, finished_ts);
   DLOG(INFO) << "minor collection finished. (" << stats.GetDuration() << ")";
   DLOG(INFO) << "minor collection stats:";
   DLOG(INFO) << " - duration: " << stats.GetDuration();
   DLOG(INFO) << " - starting utilization: " << start_stats;
   DLOG(INFO) << " - current utilization: " << finished_stats;
 }

 void Scavenger::MajorCollection(){

 }
}