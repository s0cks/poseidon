#include <glog/logging.h>
#include "poseidon/scavenger/scavenger.h"
#include "poseidon/scavenger/scavenger_serial.h"

namespace poseidon {
 static RelaxedAtomic<bool> scavenging_(false);

 bool Scavenger::IsScavenging() {
   return (bool)scavenging_;
 }

 void Scavenger::SetScavenging(bool value){
   scavenging_ = value;
 }

#define ALREADY_SCAVENGING { \
    DLOG(WARNING) << "already scavenging, skipping."; \
    return false;             \
}

 bool Scavenger::SerialScavenge(Scavenger* scavenger){
   if(IsScavenging())
     ALREADY_SCAVENGING;

   SetScavenging();
   SerialScavenger serial_scavenger(scavenger);
   TIMED_SECTION("SerialScavenge", {
     serial_scavenger.Scavenge();
   });
   ClearScavenging();
   return true;
 }

 bool Scavenger::ParallelScavenge(Scavenger* scavenger){
   NOT_IMPLEMENTED(FATAL); //TODO: implement
   return false;
 }

 bool Scavenger::Scavenge(Heap* heap){
   if(IsScavenging())
     ALREADY_SCAVENGING;

   Scavenger scavenger(heap);
   return HasWorkers() ?
          SerialScavenge(&scavenger) :
          ParallelScavenge(&scavenger);
 }

 static inline void
 CopyObject(RawObject* src, RawObject* dst) {
   VLOG_IF(10, PSDN_DEBUG) << "copying " << (*src) << " to " << (*dst);
   memcpy(dst->GetPointer(), src->GetPointer(), src->GetPointerSize());
 }

 uword Scavenger::Promote(RawObject* ptr) {
   PSDN_ASSERT(ptr->IsNew());
   PSDN_ASSERT(ptr->IsMarked());
   PSDN_ASSERT(ptr->IsRemembered());
   DLOG(INFO) << "promoting " << (*ptr);

   auto new_address = old_zone()->TryAllocate(ptr->GetPointerSize());
   if(new_address == UNALLOCATED) {
     LOG(FATAL) << "new_address == UNALLOCATED";
     return UNALLOCATED;
   }

   auto new_ptr = (RawObject*)new_address;
   CopyObject(ptr, new_ptr);
   VLOG_IF(10, google::DEBUG_MODE) << "promoted " << (*ptr) << " to " << (*new_ptr);
   //TODO: set new tag
   return new_ptr->GetStartingAddress();
 }

 uword Scavenger::Scavenge(RawObject* ptr) {
   DLOG(INFO) << "scavenging " << (*ptr);
   PSDN_ASSERT(ptr->IsNew());
   PSDN_ASSERT(ptr->IsMarked());
   PSDN_ASSERT(!ptr->IsRemembered());

   auto new_address = tospace().TryAllocate(ptr->GetPointerSize());
   if(new_address == UNALLOCATED) {
     LOG(FATAL) << "new_address == UNALLOCATED";
     return UNALLOCATED;
   }

   auto new_ptr = (RawObject*)new_address;
   CopyObject(ptr, new_ptr);
   VLOG_IF(10, google::DEBUG_MODE) << "scavenged " << (*ptr) << " to " << (*new_ptr);
   //TODO: set new tag
   return new_address;
 }

 static inline uword
 Forward(RawObject* src, RawObject* dst) {
   if(dst == UNALLOCATED)
     return src->GetStartingAddress();
   PSDN_ASSERT(!src->IsForwarding());
   DLOG(INFO) << "forwarding " << (*src) << " to " << (*dst);
   src->SetForwardingAddress(dst->GetStartingAddress());
   PSDN_ASSERT(src->IsForwarding());
   return src->GetForwardingAddress();
 }

 uword Scavenger::Process(RawObject* ptr) {
   PSDN_ASSERT(ptr->IsNew());
   PSDN_ASSERT(ptr->IsMarked());
   DLOG(INFO) << "processing " << (*ptr);

   if(ptr->IsForwarding())
     return ptr->GetForwardingAddress();

   if(ptr->IsRemembered()) {
     auto new_ptr = (RawObject*) Promote(ptr);
     new_ptr->SetRememberedBit();
     return Forward(ptr, new_ptr);
   } else {
     DLOG(INFO) << "scavenging " << (*ptr);
     auto new_ptr = (RawObject*) Scavenge(ptr);
     new_ptr->SetRememberedBit();
     return Forward(ptr, new_ptr);
   }
 }
}