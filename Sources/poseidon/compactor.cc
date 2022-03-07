#include "poseidon/finalizer.h"
#include "poseidon/allocator.h"
#include "poseidon/compactor.h"

namespace poseidon{
 void Compactor::MarkLiveObjects(){
   Allocator::VisitLocals([&](RawObject* val){
     if(page()->Contains(val->GetAddress()) && !val->IsMarked())
       MarkObject(val);
     return true;
   });
 }

 void Compactor::ComputeForwardingAddresses(){
// 1. Compute the forwarding location for live objects.
//    * Keep track of a free and live pointer and initialize both to the start of heap.
//    * If the live pointer points to a live object, update that object's forwarding pointer to the current free pointer and increment the free pointer according to the object's size.
//    * Move the live pointer to the next object
//    * End when the live pointer reaches the end of heap.
   while(live_ < page()->GetEndingAddress() && live_ptr()->GetPointerSize() > 0){
     auto ptr = live_ptr();
     if(ptr->IsMarked()){
       free_ += ForwardObject(ptr);
     }
     live_ += ptr->GetTotalSize();
   }
 }

 void Compactor::UpdateLiveObjects(){
// 1. Update all pointers
//    * For each live object, update its pointers according to the forwarding pointers of the objects they point to.
// 2. Move objects
//    * For each live object, move its data to its forwarding location.
   Allocator::VisitLocals([&](RawObject** val){
     auto old_val = (*val);
     if(old_val->IsMarked()){
#ifdef PSDN_DEBUG
       assert(old_val->IsForwarding());
#endif//PSDN_DEBUG
       (*val) = (RawObject*)old_val->GetForwardingAddress();
     }
     return true;
   });
 }

 void Compactor::ForwardAndFinalizeObjects(){
   Finalizer finalizer;

   auto next_address = page()->GetStartingAddress();
   auto current_address = page()->GetStartingAddress();
   while(current_address < page()->GetEndingAddress()){
     auto current = (RawObject*)current_address;
     if(current->GetPointerSize() <= 0)
       break;

     auto length = current->GetTotalSize();

     if(current->IsMarked()){
#ifdef PSDN_DEBUG
       assert(current->IsForwarding());
#endif//PSDN_DEBUG
       next_address += Copy(current);
     } else{
       finalizer.Finalize(current);
     }

     current_address += length;
   }
   page()->SetCurrent(next_address);

   DLOG(INFO) << "*** number of objects finalized: " << finalizer.GetNumberOfObjectsFinalized() << " (" << HumanReadableSize(finalizer.GetNumberOfBytesFinalized()) << ").";
 }

 void Compactor::Compact(){
   DLOG(INFO) << "compacting heap....";
#ifdef PSDN_DEBUG
   auto total_bytes = page()->size();
   auto start_bytes = page()->GetNumberOfBytesAllocated();
   auto start_ts = Clock::now();
#endif//PSDN_DEBUG

   MarkLiveObjects();//TODO: for testing?
   ComputeForwardingAddresses();
   UpdateLiveObjects();
   ForwardAndFinalizeObjects();

#ifdef PSDN_DEBUG
   auto finish_ts = Clock::now();
   auto finish_bytes = page()->GetNumberOfBytesAllocated();
   auto duration = (finish_ts - start_ts);
   auto difference = (start_bytes - finish_bytes);
   DLOG(INFO) << "compaction finished in " << duration << ".";
   DLOG(INFO) << "starting allocation: " << HumanReadableSize(start_bytes) << "/" << HumanReadableSize(total_bytes) << " (" << PrettyPrintPercentage(start_bytes, total_bytes) << ").";
   DLOG(INFO) << "ending allocation: " << HumanReadableSize(finish_bytes) << "/" << HumanReadableSize(total_bytes) << " (" << PrettyPrintPercentage(finish_bytes, total_bytes) << ").";
   DLOG(INFO) << "freed " << HumanReadableSize(difference) << " (" << PrettyPrintPercentage(difference, total_bytes) << ").";
#endif//PSDN_DEBUG
 }
}