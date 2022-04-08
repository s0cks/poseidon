#ifndef POSEIDON_SWEEPER_H
#define POSEIDON_SWEEPER_H

#include "poseidon/zone.h"
#include "poseidon/freelist.h"

namespace poseidon{
 class Sweeper{
  private:
   static void SerialSweep(OldPage* page);
   static void ParallelSweep(OldPage* page);
  public:
   Sweeper() = delete;
   Sweeper(const Sweeper& rhs) = delete;
   ~Sweeper() = delete;

   static void Sweep(OldPage* page);

   Sweeper& operator=(const Sweeper& rhs) = delete;
 };
}

#endif//POSEIDON_SWEEPER_H