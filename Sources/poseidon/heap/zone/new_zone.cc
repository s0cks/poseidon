#include "poseidon/object.h"
#include "poseidon/heap/zone/new_zone.h"
#include "poseidon/heap/page/new_page.h"
#include "poseidon/collector/collector.h"

namespace poseidon{
 void NewZone::SwapSpaces() {
   DLOG(INFO) << "swapping spaces";
   std::swap(fromspace_, tospace_);
 }

 NewZone* NewZone::New(const poseidon::MemoryRegion& region){
   const auto total_size = region.GetSize();
   const auto semi_size = total_size / 2;
   return new NewZone(region.GetStartingAddress(), total_size, semi_size);
 }

 Pointer* NewZone::TryAllocatePointer(const word size) {
   if(size < GetMinimumObjectSize() || size > GetMaximumObjectSize()) //TODO: cleanup
     return UNALLOCATED;

   Pointer* new_ptr = nullptr;
   if((new_ptr = fromspace_.TryAllocatePointer(size)) == UNALLOCATED) {
     PSDN_CANT_ALLOCATE(ERROR, size, (*this));
     Collector::MinorCollection();

     if((new_ptr = fromspace_.TryAllocatePointer(size)) == UNALLOCATED) {
       PSDN_CANT_ALLOCATE(FATAL, size, (*this));
     }
   }
   //TODO: mark all pages intersected by
   return new_ptr;
 }

 uword NewZone::TryAllocateBytes(const word size) {
   auto new_ptr = TryAllocatePointer(size);
   if(new_ptr == UNALLOCATED)
     return UNALLOCATED;
   return new_ptr->GetObjectPointerAddress();
 }

 uword NewZone::TryAllocateClassBytes(Class* cls) {
   return TryAllocateBytes(cls->GetAllocationSize());
 }
}