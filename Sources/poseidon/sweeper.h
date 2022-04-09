#ifndef POSEIDON_SWEEPER_H
#define POSEIDON_SWEEPER_H

#include "poseidon/zone.h"
#include "poseidon/freelist.h"

namespace poseidon{
 class Sweeper{
   friend class SerialSweeper;
   friend class ParallelSweeperTask;
  private:
   static void SetSweeping();
   static void ClearSweeping();

   static void SerialSweep();
   static void ParallelSweep();
  public:
   Sweeper() = delete;
   Sweeper(const Sweeper& rhs) = delete;
   ~Sweeper() = delete;

   static void Sweep();
   static bool IsSweeping();

   Sweeper& operator=(const Sweeper& rhs) = delete;
 };
}

#endif//POSEIDON_SWEEPER_H