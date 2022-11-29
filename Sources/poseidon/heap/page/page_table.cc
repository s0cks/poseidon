#include "poseidon/heap/page/page_table.h"

namespace poseidon {
 void PageTable::MarkAllIntersectedBy(Region* region) {
   for(auto& page : *this) {
     if(page.Intersects(*region)) {
       Mark(page.GetIndex());
     }
   }
 }
}