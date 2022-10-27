#include "poseidon/heap/new_page.h"

namespace poseidon {
#define UNALLOCATED 0 //TODO: cleanup

 bool NewPage::VisitPointers(poseidon::RawObjectVisitor* vis){
   NewPageIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(!vis->Visit(next))
       return false;
   }
   return true;
 }

 bool NewPage::VisitMarkedPointers(poseidon::RawObjectVisitor* vis){
   NewPageIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(next->IsMarked() && !vis->Visit(next))
       return false;
   }
   return true;
 }

 uword NewPage::TryAllocate(poseidon::ObjectSize size){
   auto total_size = static_cast<ObjectSize>(sizeof(RawObject) + size);
   if(size <= 0 || total_size >= GetAllocatableSize()) {
     LOG(WARNING) << "cannot allocate " << Bytes(size) << " in " << (*this);
     return UNALLOCATED;
   }

   LOG(INFO) << "allocating " << Bytes(size) << " in " << (*this);
   SetMarkedBit();
   auto ptr = new (GetCurrentAddressPointer())RawObject(ObjectTag::New(size));
   current_ += total_size;
   return ptr->GetStartingAddress();
 }
}