#ifndef POSEIDON_SERIAL_SWEEPER_H
#define POSEIDON_SERIAL_SWEEPER_H

#include "poseidon/heap/freelist.h"
#include "poseidon/sweeper/sweeper_visitor.h"

namespace poseidon {
 class SerialSweeper : public SweeperVisitor<false> {
  protected:
   bool Visit(RawObject* raw) override;
  public:
   SerialSweeper() = default;
   ~SerialSweeper() override = default;

   bool SweepPage(Page* page) override {
     TIMED_SECTION("SweepPage", {
       if(!page->VisitMarkedPointers(this))
         return false;
     });
     return true;
   }
 };
}

#endif // POSEIDON_SERIAL_SWEEPER_H