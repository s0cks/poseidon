#ifndef POSEIDON_SWEEPER_BASE_H
#define POSEIDON_SWEEPER_BASE_H

#include "poseidon/heap/zone/old_zone.h"

namespace poseidon {
 template<bool Parallel>
 class SweeperVisitor : public RawObjectVisitor, public OldPageVisitor {
  protected:
   Sweeper* sweeper_;

   explicit SweeperVisitor(Sweeper* sweeper):
    sweeper_(sweeper) {
   }

   inline Sweeper* sweeper() const {
     return sweeper_;
   }

   inline OldZone* zone() const {
     return sweeper()->zone();
   }

   bool Visit(OldPage* page) override {
     return SweepPage(page);
   }
  public:
   ~SweeperVisitor() override = default;

   virtual inline bool IsParallel() const {
     return Parallel;
   }

   virtual bool SweepPage(Page* page) = 0;

   virtual bool Sweep() {
     TIMED_SECTION("SweepOldZone", {
       if(!zone()->VisitMarkedPages(this))
         return false;
     });
     return true;
   }
 };
}

#endif // POSEIDON_SWEEPER_BASE_H