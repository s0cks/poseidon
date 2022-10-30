#ifndef POSEIDON_SWEEPER_BASE_H
#define POSEIDON_SWEEPER_BASE_H

#include "poseidon/heap/old_zone.h"

namespace poseidon {
 template<bool Parallel>
 class SweeperVisitor : public RawObjectVisitor, public OldPageVisitor {
  protected:
   SweeperVisitor() = default;

   bool Visit(OldPage* page) override {
     return SweepPage(page);
   }
  public:
   ~SweeperVisitor() override = default;

   virtual inline bool IsParallel() const {
     return Parallel;
   }

   virtual bool SweepPage(Page* page) = 0;

   virtual bool Sweep(OldZone* zone) {
     TIMED_SECTION("SweepOldZone", {
       return zone->VisitPages(this); //TODO: visit marked pages
     });
   }
 };
}

#endif // POSEIDON_SWEEPER_BASE_H