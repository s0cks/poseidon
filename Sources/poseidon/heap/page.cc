#include "poseidon/heap/page.h"

namespace poseidon {
 void Page::Clear() {
   NOT_IMPLEMENTED(ERROR); //TODO: implement
 }

 bool Page::VisitPointers(poseidon::RawObjectVisitor* vis){
   NOT_IMPLEMENTED(ERROR); //TODO: implement
   return false;
 }

 bool Page::VisitMarkedPointers(poseidon::RawObjectVisitor* vis){
   NOT_IMPLEMENTED(ERROR); //TODO: implement
   return false;
 }

 uword Page::TryAllocate(int64_t size){
   auto total_size = static_cast<uint64_t>(sizeof(RawObject) + size);
   if(!Contains(GetCurrentAddress() + total_size)) {
     PSDN_CANT_ALLOCATE(FATAL, total_size, this);
   }

   ObjectTag tag;
   tag.SetSize(size);
   auto ptr = new (GetCurrentAddressPointer())RawObject(tag);
   current_ += total_size;
   return ptr->GetStartingAddress();
 }
}