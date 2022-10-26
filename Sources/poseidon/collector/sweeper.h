#ifndef POSEIDON_SWEEPER_H
#define POSEIDON_SWEEPER_H

#include "poseidon/utils.h"
#include "poseidon/common.h"
#include "poseidon/heap/old_zone.h"
#include "poseidon/sweeper/sweeper_stats.h"

namespace poseidon{
 class OldZone;
 class Sweeper{
   friend class SweeperTest;

   friend class SerialSweeper;
   friend class ParallelSweeperTask;
  private:
   static void SetSweeping(bool active = true);

   static inline void
   ClearSweeping(){
     SetSweeping(false);
   }

   static void SweepObject(FreeList* free_list, RawObject* ptr);

   static void SerialSweep(OldZone* old_zone);
   static void ParallelSweep(OldZone* old_zone);
  public:
   Sweeper() = delete;
   Sweeper(const Sweeper& rhs) = delete;
   ~Sweeper() = delete;

   static void Sweep();
   static bool IsSweeping();

   static Timestamp GetLastSweepTimestamp();
   static int64_t GetLastSweepDurationMillis();
   static double GetLastSweepFragmentationPercentage();
   static int64_t GetNumberOfBytesLastSweep();
   static int64_t GetNumberOfObjectsLastSweep();

   static SweeperStats GetStats() {
     return {};//TODO: implement
   }

   Sweeper& operator=(const Sweeper& rhs) = delete;
 };
}

#endif//POSEIDON_SWEEPER_H