#ifndef POSEIDON_SWEEPER_H
#define POSEIDON_SWEEPER_H

#include "poseidon/heap/old_zone.h"

namespace poseidon {
 class Sweeper {
   template<bool Parallel>
   friend class SweeperVisitor;

   friend class SerialSweeper;
   friend class ParallelSweeper;
  private:
   static void SetSweeping(bool value = true);

   static inline void
   ClearSweeping() {
     return SetSweeping(false);
   }

   static bool SerialSweep(Sweeper* sweeper);
   static bool ParallelSweep(Sweeper* sweeper);
  protected:
   OldZone* zone_;

   inline OldZone* zone() const {
     return zone_;
   }

   explicit Sweeper(OldZone* zone):
    zone_(zone) {
   }

   virtual bool Sweep(Pointer* raw);
  public:
   Sweeper(const Sweeper& rhs) = delete;
   virtual ~Sweeper() = default;

   Sweeper& operator=(const Sweeper& rhs) = delete;
  public:
   static bool IsSweeping();
   static bool Sweep(OldZone* zone);
 };
}

#endif // POSEIDON_SWEEPER_H