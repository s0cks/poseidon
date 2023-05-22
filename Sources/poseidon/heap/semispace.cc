#include "poseidon/object.h"
#include "poseidon/heap/semispace.h"

namespace poseidon{
 Pointer* Semispace::TryAllocatePointer(const ObjectSize size) {
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
   current_ += new_ptr->GetTotalSize();
   DLOG(INFO) << "allocated " << Bytes(size) << " in " << (*this);
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