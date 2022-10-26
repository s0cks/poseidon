#ifndef POSEIDON_SERIAL_SWEEPER_H
#define POSEIDON_SERIAL_SWEEPER_H

#include "poseidon/heap/freelist.h"
#include "poseidon/sweeper/sweeper_base.h"

namespace poseidon {
 class SerialSweeper : public SweeperVisitor<false> {
  protected:
   FreeList* free_list_;

   inline FreeList* free_list() const {
     return free_list_;
   }

   bool Visit(RawObject* raw) override;
  public:
   explicit SerialSweeper(FreeList* free_list):
    SweeperVisitor<false>(),
    free_list_(free_list) {
   }
   ~SerialSweeper() override = default;

   bool SweepPage(Page* page) override {
     TIMED_SECTION("SweepPage", {
       if(!page->VisitPointers(this))
         return false;
     });
     return true;
   }
 };
}

#endif // POSEIDON_SERIAL_SWEEPER_H