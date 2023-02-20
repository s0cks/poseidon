#include "page_table.h"

namespace poseidon{
#define BEGIN_FOR_EACH_PAGE(Iter, NextName) \
 auto Iter = PageTablePageIterator(this);   \
 while((Iter).HasNext()) {                  \
  auto NextName = (Iter).Next();

#define END_FOR_EACH_PAGE \
 }                        \
 return true;

 bool PageTable::VisitPages(PageVisitor* vis) {
   BEGIN_FOR_EACH_PAGE(iter, next);
    if(!vis->Visit(next))
      return false;
   END_FOR_EACH_PAGE
 }

 bool PageTable::VisitMarkedPages(PageVisitor* vis) {
   BEGIN_FOR_EACH_PAGE(iter, next);
     if(IsMarked(GetPageIndex(next)) && !vis->Visit(next))
       return false;
   END_FOR_EACH_PAGE
 }

 bool PageTable::VisitPointers(RawObjectVisitor* vis) {
   return IteratePointers<PageTable, PageTablePointerIterator>(vis);
 }

 bool PageTable::VisitMarkedPointers(RawObjectVisitor* vis) {
   return IterateMarkedPointers<PageTable, PageTablePointerIterator>(vis);
 }

 void PageTable::MarkAllIntersectedBy(const Region& region) {
   VisitPages([&](Page* page) {
     if(region.Intersects((const Region&)*page))
       Mark(GetPageIndex(page));
     return true;
   });
 }
}