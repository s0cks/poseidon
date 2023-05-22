#include "old_page.h"

namespace poseidon {
 bool OldPage::VisitPointers(RawObjectVisitor* vis) const {
   return false;
 }

 bool OldPage::VisitMarkedPointers(RawObjectVisitor* vis) const {
   return false;
 }
}