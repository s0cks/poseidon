#ifndef POSEIDON_SERIAL_SWEEPER_H
#define POSEIDON_SERIAL_SWEEPER_H

#include "poseidon/heap/freelist.h"
#include "poseidon/sweeper/sweeper.h"
#include "poseidon/sweeper/sweeper_visitor.h"

namespace poseidon {
 class SerialSweeper : public SweeperVisitor<false> {
   friend class Sweeper;
   friend class SerialSweeperTest;
  protected:
   explicit SerialSweeper(Sweeper* sweeper):
     SweeperVisitor<false>(sweeper) {
   }

   bool Visit(Pointer* raw) override;
  public:
   ~SerialSweeper() override = default;

   bool SweepPage(Page* page) override;
 };
}

#endif // POSEIDON_SERIAL_SWEEPER_H