#include "poseidon/heap/semispace.h"

namespace poseidon{
 uword Semispace::TryAllocate(int64_t size){
   auto total_size = static_cast<int64_t>(sizeof(RawObject) + size);
   if((GetCurrentAddress() + total_size) > GetEndingAddress()){
     PSDN_CANT_ALLOCATE(ERROR, total_size, (*this));
   }

   auto ptr = new (GetCurrentAddressPointer())RawObject(ObjectTag::NewWithSize(size));
   current_ += total_size;
   return ptr->GetAddress();
 }
}