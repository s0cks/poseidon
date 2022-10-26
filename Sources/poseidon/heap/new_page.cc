#include "poseidon/heap/new_page.h"

namespace poseidon {
#define UNALLOCATED 0 //TODO: cleanup

 uword NewPage::TryAllocate(poseidon::ObjectSize size){
   if(size <= 0) {
     LOG(WARNING) << "cannot allocate " << Bytes(size) << " in " << (*this);
     return UNALLOCATED;
   }

   if(size >= GetAllocatableSize()) {
     LOG(WARNING) << "cannot allocate " << Bytes(size) << " in " << (*this);
     return UNALLOCATED;
   }

   auto total_size = static_cast<ObjectSize>(sizeof(RawObject) + size);
   if(!Contains(GetCurrentAddress() + total_size)) {
     LOG(ERROR) << "failed to allocate " << Bytes(total_size) << " in " << (*this);
     return 0;
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