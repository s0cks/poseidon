#include "poseidon/heap/page/old_page.h"

namespace poseidon {
#define UNALLOCATED 0 //TODO: cleanup

 bool OldPage::VisitPointers(RawObjectVisitor* vis){
   return false;
 }

 bool OldPage::VisitMarkedPointers(RawObjectVisitor* vis){
   return false;
 }
}