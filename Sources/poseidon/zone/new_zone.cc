#include "poseidon/object.h"
#include "new_zone.h"
#include "poseidon/page/new_page.h"
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

 Pointer* NewZone::TryAllocatePointer(const ObjectSize size) {
   if(size < NewZone::GetMinimumObjectSize() || size > NewZone::GetMaximumObjectSize()) {
     PSDN_CANT_ALLOCATE(ERROR, size, (*this));
     return UNALLOCATED;
   }

   auto total_size = size + static_cast<ObjectSize>(sizeof(Pointer));
   if((fromspace_ + total_size) >= tospace_) {
     PSDN_CANT_ALLOCATE(ERROR, size, (*this));
     return UNALLOCATED;
   }

   DLOG(INFO) << "allocating " << Bytes(total_size) << " in " << (*this);
   auto new_ptr = Pointer::New(fromspace(), PointerTag::New(size));
   memset((void*)new_ptr->GetObjectPointerAddress(), 0, new_ptr->GetSize());
   fromspace_ += new_ptr->GetTotalSize();
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