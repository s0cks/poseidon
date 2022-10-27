#include "poseidon/heap/old_page.h"

namespace poseidon {
#define UNALLOCATED 0 //TODO: cleanup

 bool OldPage::VisitPointers(poseidon::RawObjectVisitor* vis){
   OldPageIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(!vis->Visit(next))
       return false;
   }
   return true;
 }

 bool OldPage::VisitMarkedPointers(poseidon::RawObjectVisitor* vis){
   OldPageIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(next->IsMarked() && !vis->Visit(next))
       return false;
   }
   return true;
 }

 uword OldPage::TryAllocate(int64_t size) {
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
  tag.SetOld();
  auto ptr = new (GetCurrentAddressPointer())RawObject(tag);
  current_ += total_size;
  return ptr->GetStartingAddress();
  }
}