#include "free_ptr.h"

namespace poseidon {
 int FreePointer::Compare(const FreePointer& lhs, const FreePointer& rhs){
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

 FreePointer* FreePointer::From(const Region& region){
   if(region.GetStartingAddress() == 0 || region.GetSize() == 0 || region.GetSize() > flags::GetOldZoneSize()) {
     DLOG(ERROR) << "cannot create FreePointer from " << region;
     return nullptr;
   }
   return new (region.GetStartingAddressPointer())FreePointer(PointerTag::OldFree(region.GetSize()));
 }
}