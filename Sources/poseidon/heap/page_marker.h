#ifndef POSEIDON_PAGE_MARKER_H
#define POSEIDON_PAGE_MARKER_H

#include "poseidon/heap/page_table.h"

namespace poseidon {
 class PageMarker : public PageVisitor {
  protected:
   PageTable* table_;
  public:
   explicit PageMarker(PageTable* table):
    PageVisitor(),
    table_(table) {
   }
   ~PageMarker() override = default;

   inline PageTable* table() const {
     return table_;
   }

   bool VisitPage(Page* page) override {
     LOG(INFO) << "marking " << (*page);
     table()->Mark(page);
     return true;
   }
 };
}

#endif // POSEIDON_PAGE_MARKER_H