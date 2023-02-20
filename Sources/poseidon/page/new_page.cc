#include "new_page.h"

namespace poseidon {
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