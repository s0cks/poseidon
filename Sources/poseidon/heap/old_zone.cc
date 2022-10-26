#include "poseidon/heap/old_zone.h"

namespace poseidon{
#define UNALLOCATED 0 //TODO: cleanup

 uword OldZone::TryAllocate(const ObjectSize size) {
   if(size <= 0 || size >= GetAllocatableSize()) {
     LOG(WARNING) << "cannot allocate " << Bytes(size) << " in " << (*this);
     return UNALLOCATED;
   }

   auto ptr = (void*)free_list()->TryAllocate(size);
   auto val = new (ptr)RawObject();
   val->SetOldBit();
   val->SetPointerSize(size);
   return val->GetStartingAddress();
 }
}