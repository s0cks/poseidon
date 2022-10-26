#include "poseidon/heap/page_table.h"
#include "poseidon/heap/page_marker.h"

namespace poseidon {
 void PageTable::MarkAll(const Region& region) {
   PageMarker marker(this);
   PageTableIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if ((next->Contains(region) || region.Contains(*next)) && !marker.Visit(next))
       return;
   }
 }

 void PageTable::VisitPages(poseidon::PageVisitor* vis) const {
   PageTableIterator iterator(this);
   while(iterator.HasNext()) {
     if(!vis->Visit(iterator.Next()))
       return;
   }
 }

 void PageTable::VisitMarkedPages(poseidon::PageVisitor* vis) const{
   PageTableIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(next->marked() && !vis->Visit(next))
       return;
   }
 }

 void PageTable::VisitUnmarkedPages(poseidon::PageVisitor* vis) const{
   PageTableIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(!next->marked() && !vis->Visit(next))
       return;
   }
 }
}