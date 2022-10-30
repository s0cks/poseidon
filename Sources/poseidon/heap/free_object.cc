#include "poseidon/heap/free_object.h"

namespace poseidon {
 int FreeObject::Compare(const FreeObject& lhs, const FreeObject& rhs){
   if(lhs.GetSize() > rhs.GetSize())
     return -1;
   else if(lhs.GetSize() < rhs.GetSize())
     return +1;

   if(lhs.GetStartingAddress() < rhs.GetStartingAddress())
     return -1;
   else if(lhs.GetStartingAddress() > rhs.GetStartingAddress())
     return +1;
   return 0;
 }

 FreeObject* FreeObject::From(const Region& region){
   if(region.GetStartingAddress() == 0 || region.GetSize() == 0 || region.GetSize() > GetOldZoneSize())
     return nullptr;
   return new (region.GetStartingAddressPointer())FreeObject(ObjectTag::OldFree(region.GetSize()));
 }
}