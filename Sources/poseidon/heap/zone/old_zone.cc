#include "poseidon/heap/zone/old_zone.h"
#include "poseidon/heap/page_marker.h"
#include "poseidon/collector/collector.h"

namespace poseidon{
 uword OldZone::TryAllocateBytes(const word size) { //TODO: cleanup
   if(size < GetMinimumObjectSize() || size > GetMaximumObjectSize()) {
     LOG(WARNING) << "cannot allocate " << Bytes(size) << " in " << (*this);
     return UNALLOCATED;
   }

   uword ptr_address = UNALLOCATED;
   if((ptr_address = free_list_.TryAllocate(size)) == UNALLOCATED) {
     PSDN_CANT_ALLOCATE(ERROR, size, (*this));
     Collector::MajorCollection();
     if((ptr_address = free_list_.TryAllocate(size)) == UNALLOCATED) {
       PSDN_CANT_ALLOCATE(FATAL, size, (*this));
     }
   }

   //TODO: mark all pages intersected by
   auto val = new ((void*) ptr_address)Pointer(PointerTag::Old(size));
   memset((void*) val->GetObjectPointerAddress(), 0, val->GetSize());
   return val->GetObjectPointerAddress();
 }
}