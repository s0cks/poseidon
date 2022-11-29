#include "poseidon/heap/heap.h"
#include "poseidon/type/class.h"
#include "poseidon/heap/semispace.h"

namespace poseidon{
 void Semispace::Clear() {
   memset(GetStartingAddressPointer(), 0, GetSize());
   current_ = GetStartingAddress();
 }

 Pointer* Semispace::TryAllocatePointer(const word size) {
   if(size < GetMinimumObjectSize() || size > GetMaximumObjectSize()) {
     PSDN_CANT_ALLOCATE(ERROR, size, (*this));
     return UNALLOCATED;
   }

   auto total_size = GetTotalSizeNeededFor(size);
   if((GetCurrentAddress() + total_size) > GetEndingAddress()) {
     PSDN_CANT_ALLOCATE(ERROR, total_size, (*this));
     return UNALLOCATED;
   }

   auto new_ptr = new (GetCurrentAddressPointer())Pointer(PointerTag::New(size));
   memset((void*) new_ptr->GetObjectPointerAddress(), 0, new_ptr->GetSize());
   current_ += total_size;
   return new_ptr;
 }

 uword Semispace::TryAllocateBytes(const word size) {
   auto new_ptr = TryAllocatePointer(size);
   if(new_ptr == UNALLOCATED)
     return UNALLOCATED;
   return new_ptr->GetObjectPointerAddress();
 }

 uword Semispace::TryAllocateClassBytes(Class* cls) {
   return TryAllocateBytes(cls->GetAllocationSize()); //TODO: refactor
 }
}