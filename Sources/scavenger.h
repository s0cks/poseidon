#ifndef POSEIDON_SCAVENGER_H
#define POSEIDON_SCAVENGER_H

#include "raw_object.h"

namespace poseidon{
  class Heap;
  class Scavenger{ //TODO: remove static
   private:
    static void MarkLiveObjects();
    static void PromoteLiveObjects();
    static void ScavengeLiveObjects(Heap* heap);
    static void UpdateForwarding();

    static void ScavengeFromEdenHeap();
    static void ScavengeFromTenuredHeap();
    static void ScavengeFromLargeObjectSpace();
   public:
    Scavenger() = default;
    ~Scavenger() = default;

    static void Scavenge();
    static void MajorCollection();
    static void MinorCollection();
  };
}

#endif //POSEIDON_SCAVENGER_H
