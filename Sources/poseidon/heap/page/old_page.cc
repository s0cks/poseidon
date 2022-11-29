#include "poseidon/heap/page/old_page.h"

namespace poseidon {
#define UNALLOCATED 0 //TODO: cleanup

 bool OldPage::VisitPointers(RawObjectVisitor* vis){
   OldPageIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(next->IsOld() && !next->IsFree() && !vis->Visit(next))
       return false;
   }
   return true;
 }

 bool OldPage::VisitMarkedPointers(RawObjectVisitor* vis){
   OldPageIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(next->IsOld() && !next->IsFree() && next->IsMarked() && !vis->Visit(next))
       return false;
   }
   return true;
 }
}