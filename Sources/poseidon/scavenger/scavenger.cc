#include <glog/logging.h>
#include "poseidon/scavenger/scavenger.h"
#include "poseidon/scavenger/scavenger_serial.h"

namespace poseidon {
 static RelaxedAtomic<Scavenger::State> state_;

 Scavenger::State Scavenger::GetState() {
   return (State)state_;
 }

 void Scavenger::SetState(const State& state) {
   state_ = state;
 }

#define ALREADY_SCAVENGING { \
    DLOG(WARNING) << "already scavenging, skipping."; \
    return false;             \
}

 bool Scavenger::SerialScavenge(Scavenger* scavenger){
   if(IsProcessingRoots() || IsProcessingToSpace())
     ALREADY_SCAVENGING;

   //TODO: check heap state of current thread
   auto heap = Heap::GetForCurrentThread();
   SerialScavenger serial_scavenger(scavenger, heap->new_zone(), heap->old_zone());
   TIMED_SECTION("SerialScavenge", {
     serial_scavenger.ScavengeMemory();
   });
   SetIdle();
   return true;
 }

 bool Scavenger::ParallelScavenge(Scavenger* scavenger){
   NOT_IMPLEMENTED(FATAL); //TODO: implement
   return false;
 }

 bool Scavenger::ScavengeMemory(Heap* heap){
   if(IsProcessingRoots() || IsProcessingToSpace())
     ALREADY_SCAVENGING;

   Scavenger scavenger(heap->new_zone(), heap->old_zone());
   return flags::FLAGS_num_workers > 0 ?
          SerialScavenge(&scavenger) :
          ParallelScavenge(&scavenger);
 }

 static inline void
 CopyObject(Pointer* src, Pointer* dst) {
   DLOG(INFO) << "copying " << (*src) << " to " << (*dst);
   memcpy(dst->GetPointer(), src->GetPointer(), src->GetPointerSize());
 }

 static inline uword
 Forward(Pointer* src, Pointer* dst) {
   PSDN_ASSERT(!src->IsForwarding());
   DLOG(INFO) << "forwarding " << (*src) << " to " << (*dst);
   src->SetForwardingAddress(dst->GetStartingAddress());
   PSDN_ASSERT(src->IsForwarding());
   return src->GetForwardingAddress();
 }

 uword Scavenger::Promote(Pointer* ptr) {
   LOG(INFO) << "promoting " << (*ptr);
   PSDN_ASSERT(ptr->IsNew());
   PSDN_ASSERT(ptr->IsMarked());
   PSDN_ASSERT(ptr->IsRemembered());

   auto new_address = old_zone()->TryAllocatePointer(ptr->GetPointerSize());
   if(new_address == UNALLOCATED) {
     LOG(FATAL) << "new_address == UNALLOCATED";
     return UNALLOCATED;
   }

   auto new_ptr = (Pointer*)new_address;
   DLOG(INFO) << "new-ptr: " << new_ptr;
   CopyObject(ptr, new_ptr);
   VLOG_IF(10, google::DEBUG_MODE) << "promoted " << (*ptr) << " to " << (*new_ptr);
   new_ptr->SetRemembered();
   return Forward(ptr, new_ptr);
 }

 Pointer* Scavenger::TryAllocate(const ObjectSize size) {
   NOT_IMPLEMENTED(FATAL); //TODO: implement
   return UNALLOCATED;
 }

 uword Scavenger::Scavenge(Pointer* ptr) {
   PSDN_ASSERT(ptr->IsNew());
   PSDN_ASSERT(ptr->IsMarked());
   PSDN_ASSERT(!ptr->IsRemembered());

   auto size = ptr->GetPointerSize();
   auto new_ptr = (Pointer*)new_zone()->TryAllocatePointer(size);
   if(new_ptr == UNALLOCATED) {
     LOG(FATAL) << "new_address == UNALLOCATED";
     return UNALLOCATED;
   }

   CopyObject(ptr, new_ptr);
   new_ptr->SetRemembered();
   new_ptr->SetTypeId(ptr->GetTypeId());
   DLOG(INFO) << "scavenged " << (*ptr) << " to " << (*new_ptr);
   return Forward(ptr, new_ptr);
 }
}