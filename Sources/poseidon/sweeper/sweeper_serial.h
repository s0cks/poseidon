#ifndef POSEIDON_SERIAL_SWEEPER_H
#define POSEIDON_SERIAL_SWEEPER_H

#include "poseidon/freelist/freelist.h"
#include "poseidon/sweeper/sweeper.h"
#include "poseidon/sweeper/sweeper_visitor.h"

namespace poseidon {
 class SerialSweeper : public SweeperVisitor<false> {
   friend class Sweeper;
   friend class SerialSweeperTest;
  protected:
   SerialSweeper(Sweeper* sweeper, OldZone* zone):
     SweeperVisitor<false>(sweeper, zone) {
   }

   bool Visit(Pointer* raw) override;
  public:
   ~SerialSweeper() override = default;
 };
}

#endif // POSEIDON_SERIAL_SWEEPER_H