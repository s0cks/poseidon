#include "poseidon/heap/zone/new_zone.h"
#include "poseidon/heap/page/new_page.h"
#include "poseidon/heap/page_marker.h"
#include "poseidon/collector/collector.h"

namespace poseidon{
#define UNALLOCATED 0 //TODO: cleanup

 void NewZone::SwapSpaces() {
   DLOG(INFO) << "swapping spaces";
   std::swap(fromspace_, tospace_);
 }

  bool NewZone::MarkAllIntersectedBy(const Region& region){
    return PageMarker<NewZone>::MarkAllIntersectedBy(this, region);
  }

 NewZone* NewZone::New(const poseidon::MemoryRegion& region){
   const auto total_size = region.GetSize();
   const auto semi_size = total_size / 2;
   return new NewZone(region.GetStartingAddress(), total_size, semi_size);
 }

 uword NewZone::TryAllocateBytes(const int64_t size) {
   if(size < GetMinimumObjectSize() || size > GetMaximumObjectSize()) //TODO: cleanup
     return UNALLOCATED;

   uword ptr_address = UNALLOCATED;
   if((ptr_address = fromspace_.TryAllocateBytes(size)) == UNALLOCATED) {
     PSDN_CANT_ALLOCATE(ERROR, size, (*this));
     Collector::MinorCollection();

     if((ptr_address = fromspace_.TryAllocateBytes(size)) == UNALLOCATED) {
       PSDN_CANT_ALLOCATE(FATAL, size, (*this));
     }
   }
   auto ptr = (Pointer*)ptr_address;
   MarkAllIntersectedBy(*ptr);
   memset((void*) ptr->GetObjectPointerAddress(), 0, ptr->GetSize());
   return ptr->GetObjectPointerAddress();
 }
}