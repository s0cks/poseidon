#ifndef POSEIDON_SWEEPER_H
#define POSEIDON_SWEEPER_H

#include "poseidon/heap/zone/old_zone.h"

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

   static bool SerialSweep(Sweeper* sweeper, OldZone* zone);
   static bool ParallelSweep(Sweeper* sweeper, OldZone* zone);
  protected:
   Sweeper() = default;
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