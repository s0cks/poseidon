#include "poseidon/heap/page.h"

namespace poseidon {
 uword Page::TryAllocate(int64_t size){
   auto total_size = static_cast<uint64_t>(sizeof(RawObject) + size);
   if(!Contains(GetCurrentAddress() + total_size)) {
     PSDN_CANT_ALLOCATE(FATAL, total_size, this);
   }

   ObjectTag tag;
   tag.SetSize(size);
   auto ptr = new (GetCurrentAddressPointer())RawObject(tag);
   current_ += total_size;
   return ptr->GetStartingAddress();
 }
}