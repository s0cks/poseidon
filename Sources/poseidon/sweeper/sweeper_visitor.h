#ifndef POSEIDON_SWEEPER_BASE_H
#define POSEIDON_SWEEPER_BASE_H

#include "poseidon/heap/zone/old_zone.h"

namespace poseidon {
 template<bool Parallel>
 class SweeperVisitor : public RawObjectVisitor {
  protected:
   Sweeper* sweeper_;

   explicit SweeperVisitor(Sweeper* sweeper):
    sweeper_(sweeper) { }

   inline Sweeper* sweeper() const {
     return sweeper_;
   }
  public:
   ~SweeperVisitor() override = default;

   virtual inline bool IsParallel() const {
     return Parallel;
   }

   virtual bool Sweep() {
     TIMED_SECTION("Sweep", {
//       if(!zone()->VisitMarkedPages(this))
//         return false;
     });
     return true;
   }
 };
}

#endif // POSEIDON_SWEEPER_BASE_H