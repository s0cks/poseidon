#include "poseidon/heap/heap.h"
#include "poseidon/type/class.h"
#include "poseidon/heap/semispace.h"

namespace poseidon{
#define UNALLOCATED 0 //TODO: cleanup

 void Semispace::Clear() {
   memset(GetStartingAddressPointer(), 0, GetSize());
   current_ = GetStartingAddress();
 }

 bool Semispace::VisitPointers(RawObjectVisitor* vis){
   SemispaceIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(!vis->Visit(next))
       return false;
   }
   return true;
 }

 bool Semispace::VisitPointers(std::function<bool(Pointer*)>& function){
   SemispaceIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(!function(next))
       return false;
   }
   return true;
 }

 bool Semispace::VisitMarkedPointers(RawObjectVisitor* vis){
   SemispaceIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(next->IsMarked() && !vis->Visit(next))
       return false;
   }
   return true;
 }

 bool Semispace::VisitMarkedPointers(std::function<bool(Pointer*)>& function){
   SemispaceIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(next->IsMarked() && !function(next))
       return false;
   }
   return true;
 }

 uword Semispace::TryAllocateBytes(ObjectSize size) {
   if(size < GetMinimumObjectSize() || size > GetMaximumObjectSize()) {
     PSDN_CANT_ALLOCATE(ERROR, size, (*this));
     return UNALLOCATED;
   }

   auto total_size = GetTotalSizeNeededFor(size);
   if((GetCurrentAddress() + total_size) > GetEndingAddress()) {
     PSDN_CANT_ALLOCATE(ERROR, total_size, (*this));
     return UNALLOCATED;
   }

   auto new_ptr = new (GetCurrentAddressPointer())Pointer(PointerTag::New(size));
   current_ += total_size;
   return new_ptr->GetStartingAddress();
 }

 uword Semispace::TryAllocateClassBytes(Class* cls) {
   return TryAllocateBytes(cls->GetAllocationSize()); //TODO: refactor
 }
}