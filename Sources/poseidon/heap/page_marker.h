#ifndef POSEIDON_PAGE_MARKER_H
#define POSEIDON_PAGE_MARKER_H

#include "poseidon/heap/page/page.h"
#include "poseidon/heap/page/old_page.h"
#include "poseidon/heap/page/new_page.h"

namespace poseidon {
 template<class Z>
 class PageMarker : public NewPageVisitor, public OldPageVisitor {
   friend class NewZone;
   friend class OldZone;
  protected:
   Z* zone_;
   const Region& region_;

   inline Z* zone() const {
     return zone_;
   }

   inline const Region& region() const {
     return region_;
   }

   explicit PageMarker(Z* zone, const Region& region):
    NewPageVisitor(),
    OldPageVisitor(),
    zone_(zone),
    region_(region) {
   }
  public:
   ~PageMarker() override = default;

   bool Visit(NewPage* page) override {
     if(region().Intersects((*page)))
       zone()->Mark(page->GetIndex());
     return true;
   }

   bool Visit(OldPage* page) override {
     if(region().Intersects((*page)))
       zone()->Mark(page->GetIndex());
     return true;
   }

   static inline bool
   MarkAllIntersectedBy(Z* zone, const Region& region) {
     PageMarker<Z> marker(zone, region);
     return zone->VisitPages(&marker);
   }
 };
}

#endif // POSEIDON_PAGE_MARKER_H