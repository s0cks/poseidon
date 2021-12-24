#ifndef POSEIDON_SCAVENGER_H
#define POSEIDON_SCAVENGER_H

#include "raw_object.h"

namespace poseidon{
  class Heap;
  class Scavenger{
   public:
    Scavenger() = default;
    ~Scavenger() = default;

    void MarkLiveObjects();
    void PromoteLiveObjects();
    void ScavengeLiveObjects(Heap* heap);
    void UpdateLocals();
    void Scavenge();
  };
}

#endif //POSEIDON_SCAVENGER_H
