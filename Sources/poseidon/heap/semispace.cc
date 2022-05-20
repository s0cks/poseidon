#include "poseidon/heap/semispace.h"

namespace poseidon{
 uword Semispace::TryAllocate(int64_t size){
   auto total_size = static_cast<int64_t>(sizeof(RawObject) + size);
   if((((uword)current_) + total_size) > GetEndingAddress()){
     DLOG(ERROR) << "cannot allocate object of " << Bytes(total_size) << " in " << (*this);
     return 0;
   }

   auto ptr = new ((void*)((uword)current_))RawObject();
   current_ += total_size;

   ptr->ClearNewBit();
   ptr->ClearOldBit();
   ptr->ClearMarkedBit();
   ptr->ClearRememberedBit();
   ptr->SetPointerSize(size);//TODO: make one atomic store
   return ptr->GetAddress();
 }
}