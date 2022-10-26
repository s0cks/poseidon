#include "poseidon/heap/old_zone.h"

namespace poseidon{
 uword OldZone::TryAllocate(int64_t size) {
   auto ptr = (void*)free_list()->TryAllocate(size);
   auto val = new (ptr)RawObject();
   val->SetOldBit();
   val->SetPointerSize(size);
   return val->GetStartingAddress();
 }
}