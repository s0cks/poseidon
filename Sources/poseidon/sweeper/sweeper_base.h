#ifndef POSEIDON_SWEEPER_BASE_H
#define POSEIDON_SWEEPER_BASE_H

#include "poseidon/heap/page.h"

namespace poseidon {
 template<bool Parallel>
 class SweeperBase : public RawObjectVisitor, public PageVisitor {
  protected:
   SweeperBase() = default;
  public:
   ~SweeperBase() override = default;

   virtual inline bool IsParallel() const {
     return Parallel;
   }

   inline bool Sweep(Page* page) {
     TIMED_SECTION("SweepPage", {
       return PageVisitor::VisitPage(page);
     });
   }
 };
}

#endif // POSEIDON_SWEEPER_BASE_H