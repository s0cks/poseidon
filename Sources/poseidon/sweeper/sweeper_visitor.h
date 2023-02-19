#ifndef POSEIDON_SWEEPER_BASE_H
#define POSEIDON_SWEEPER_BASE_H

#include "poseidon/zone/old_zone.h"

namespace poseidon {
 template<bool Parallel>
 class SweeperVisitor : public RawObjectVisitor {
  protected:
   Sweeper* sweeper_;
   OldZone* zone_;

   explicit SweeperVisitor(Sweeper* sweeper, OldZone* zone):
    sweeper_(sweeper),
    zone_(zone) { }

   inline Sweeper* sweeper() const {
     return sweeper_;
   }

   inline OldZone* zone() const {
     return zone_;
   }
  public:
   ~SweeperVisitor() override = default;

   virtual inline bool IsParallel() const {
     return Parallel;
   }

   virtual bool Sweep() {
     TIMED_SECTION("Sweep", {
       ((Section*)zone())->VisitUnmarkedPointers(this);
     });
     return true;
   }
 };
}

#endif // POSEIDON_SWEEPER_BASE_H