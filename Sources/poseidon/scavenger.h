#ifndef POSEIDON_SCAVENGER_H
#define POSEIDON_SCAVENGER_H

#include "poseidon/zone.h"
#include "poseidon/heap.h"
#include "poseidon/stats.h"
#include "poseidon/utils.h"
#include "poseidon/task_pool.h"

namespace poseidon{
 class Scavenger{
   friend class ParallelScavengerTask;
   friend class SerialScavenger;
  private:
   static void SetScavenging();
   static void ClearScavenging();
  public:
   Scavenger() = delete;
   Scavenger(const Scavenger& rhs) = delete;
   ~Scavenger() = delete;

   static bool IsScavenging();
   static void Scavenge();
   static void SerialScavenge();
   static void ParallelScavenge();

   Scavenger& operator=(const Scavenger& rhs) = delete;
 };
}

#endif //POSEIDON_SCAVENGER_H
