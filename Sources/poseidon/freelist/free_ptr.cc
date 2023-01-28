#include "free_ptr.h"

namespace poseidon {
 FreePointer* FreePointer::From(const Region& region){
   if(region.GetStartingAddress() == 0 || region.GetSize() == 0 || region.GetSize() > flags::GetOldZoneSize()) {
     DLOG(ERROR) << "cannot create FreePointer from " << region;
     return nullptr;
   }
   return new (region.GetStartingAddressPointer())FreePointer(PointerTag::OldFree(region.GetSize()));
 }
}