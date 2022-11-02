#include "poseidon/heap/old_zone.h"
#include "poseidon/heap/page_marker.h"
#include "poseidon/collector/collector.h"

namespace poseidon{
#define UNALLOCATED 0 //TODO: cleanup

 bool OldZone::MarkAllIntersectedBy(const poseidon::Region& region){
   return PageMarker<OldZone>::MarkAllIntersectedBy(this, region);
 }

 OldZone* OldZone::From(const MemoryRegion& region){
   return new OldZone(region.GetStartingAddress(), region.GetSize());
 }

 uword OldZone::TryAllocate(const ObjectSize& size) { //TODO: cleanup
   if(size <= 0 || size >= GetSize()) {
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

   auto val = new ((void*) ptr_address)RawObject(ObjectTag::Old(size));
   if(!MarkAllIntersectedBy((*val))) {
     PSDN_CANT_ALLOCATE(ERROR, size, (*this));
     //TODO: deallocate
     return UNALLOCATED;
   }
   return val->GetStartingAddress();
 }
}