#include "poseidon/zone_old.h"

namespace poseidon{
 uword OldZone::TryAllocate(int64_t size){
   auto ptr = (void*)free_list_.TryAllocate(size);
   auto val = new (ptr)RawObject();
   val->SetOldBit();
   val->SetPointerSize(size);//TODO: mark page
   return val->GetAddress();
 }
}