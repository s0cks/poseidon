#include "poseidon/object.h"
#include "new_zone.h"
#include "poseidon/page/new_page.h"
#include "poseidon/collector/collector.h"

namespace poseidon{
 void NewZone::SwapSpaces() {
   DLOG(INFO) << "swapping spaces";
   DLOG(INFO) << "fromspace (before): " << fromspace();
   DLOG(INFO) << "tospace (before): " << tospace();
   std::swap(fromspace_, tospace_);
   DLOG(INFO) << "fromspace (after): " << fromspace();
   DLOG(INFO) << "tospace (after): " << tospace();
 }

 NewZone* NewZone::New(const poseidon::MemoryRegion& region){
   const auto total_size = region.GetSize();
   const auto semi_size = total_size / 2;
   return new NewZone(region.GetStartingAddress(), total_size, semi_size);
 }

 Pointer* NewZone::TryAllocatePointer(const ObjectSize size) {
   if(size < NewZone::GetMinimumObjectSize() || size > NewZone::GetMaximumObjectSize()) {
     PSDN_CANT_ALLOCATE(ERROR, size, (*this));
     return UNALLOCATED;
   }

   Pointer* new_ptr = UNALLOCATED;
   if((new_ptr = fromspace().TryAllocatePointer(size)) == UNALLOCATED) {
     PSDN_CANT_ALLOCATE(ERROR, size, (*this));
     return UNALLOCATED;
   }
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