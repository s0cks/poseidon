#include "poseidon/heap/heap.h"
#include "poseidon/heap/semispace.h"

namespace poseidon{
#define UNALLOCATED 0 //TODO: cleanup

 void Semispace::Clear() {
   memset(GetStartingAddressPointer(), 0, GetSize());
   current_ = GetStartingAddress();
 }

 bool Semispace::VisitPointers(poseidon::RawObjectVisitor* vis){
   SemispaceIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(!vis->Visit(next))
       return false;
   }
   return true;
 }

 bool Semispace::VisitMarkedPointers(poseidon::RawObjectVisitor* vis){
   SemispaceIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(next->IsMarked() && !vis->Visit(next))
       return false;
   }
   return true;
 }

 uword Semispace::TryAllocate(int64_t size){
   if(size <= 0 || size >= GetAllocatableSize())
     return UNALLOCATED;

   auto total_size = static_cast<int64_t>(sizeof(RawObject) + size);
   if((GetCurrentAddress() + total_size) > GetEndingAddress()){
     PSDN_CANT_ALLOCATE(ERROR, total_size, (*this));
   }
   auto ptr = new (GetCurrentAddressPointer())RawObject(ObjectTag::New(size));
   current_ += ptr->GetTotalSize();
   return ptr->GetStartingAddress();
 }
}