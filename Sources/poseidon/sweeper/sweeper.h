#ifndef POSEIDON_SWEEPER_H
#define POSEIDON_SWEEPER_H

#include "poseidon/heap/old_zone.h"

namespace poseidon {
 class Sweeper {
   friend class SerialSweeper;
  protected:
   static void SetSweeping(bool value = true);

   static inline void
   ClearSweeping() {
     return SetSweeping(false);
   }

   static bool SerialSweep(OldZone& zone);
   static bool ParallelSweep(OldZone& zone);
   static bool SweepObject(FreeList* free_list, RawObject* raw);
  public:
   Sweeper() = delete;
   Sweeper(const Sweeper& rhs) = delete;
   ~Sweeper() = delete;

   static bool IsSweeping();

   static inline bool
   Sweep(OldZone& zone) {
     if(IsSweeping()){
       DLOG(WARNING) << "already sweeping";
       return false;
     }

     return HasWorkers() ?
            ParallelSweep(zone) :
            SerialSweep(zone);
   }
 };
}

#endif // POSEIDON_SWEEPER_H