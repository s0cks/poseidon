#include "poseidon/heap/new_page.h"

namespace poseidon {
#define UNALLOCATED 0 //TODO: cleanup

 NewPage* NewPage::New(const MemoryRegion& region){
   return new (region.GetStartingAddressPointer())NewPage();
 }

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
}