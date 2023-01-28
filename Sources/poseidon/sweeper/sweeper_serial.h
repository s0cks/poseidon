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
   FreeList* free_list_;

   SerialSweeper(Sweeper* sweeper, FreeList* free_list):
     SweeperVisitor<false>(sweeper),
     free_list_(free_list) {
   }

   inline FreeList* free_list() const {
     return free_list_;
   }

   bool Visit(Pointer* raw) override;
  public:
   ~SerialSweeper() override = default;
 };
}

#endif // POSEIDON_SERIAL_SWEEPER_H