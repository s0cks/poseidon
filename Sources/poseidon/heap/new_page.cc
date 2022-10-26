#include "poseidon/heap/new_page.h"

namespace poseidon {
 uword NewPage::TryAllocate(poseidon::ObjectSize size){
   LOG(INFO) << "allocating " << Bytes(size) << " in " << (*this);
   auto total_size = static_cast<ObjectSize>(sizeof(RawObject) + size);
   if(!Contains(GetCurrentAddress() + total_size)) {
     LOG(ERROR) << "failed to allocate " << Bytes(total_size) << " in " << (*this);
     return 0;
   }

   SetMarkedBit();
   ObjectTag tag;
   tag.SetSize(size);
   tag.ClearMarked();
   tag.SetNew();
   auto ptr = new (GetCurrentAddressPointer())RawObject(tag);
   current_ += total_size;
   return ptr->GetStartingAddress();
 }
}