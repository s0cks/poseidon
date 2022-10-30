#ifndef POSEIDON_PAGE_MARKER_H
#define POSEIDON_PAGE_MARKER_H

#include "poseidon/heap/page.h"
#include "poseidon/heap/new_page.h"
#include "poseidon/heap/old_page.h"

namespace poseidon {
 template<class Z>
 class PageMarker : public PageVisitor {
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
    PageVisitor(),
    zone_(zone),
    region_(region) {
   }
  public:
   ~PageMarker() override = default;

   bool Visit(Page* page) override {
     if(region().Intersects((*page))) {
       if(!zone_->MarkPage(page))
         return false;
     }
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