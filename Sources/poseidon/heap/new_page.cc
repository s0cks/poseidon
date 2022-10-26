#include "poseidon/heap/new_page.h"

namespace poseidon {
#define UNALLOCATED 0 //TODO: cleanup

 uword NewPage::TryAllocate(poseidon::ObjectSize size){
   auto total_size = static_cast<ObjectSize>(sizeof(RawObject) + size);
   if(size <= 0 || total_size >= GetAllocatableSize()) {
     LOG(WARNING) << "cannot allocate " << Bytes(size) << " in " << (*this);
     return UNALLOCATED;
   }

   LOG(INFO) << "allocating " << Bytes(size) << " in " << (*this);
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