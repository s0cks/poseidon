#ifndef POSEIDON_SCAVENGER_H
#define POSEIDON_SCAVENGER_H

#include "raw_object.h"

namespace poseidon{
  class Heap;
  class Scavenger{ //TODO: remove static
   private:
    static void UpdateForwarding();
   public:
    Scavenger() = default;
    ~Scavenger() = default;

    static void MajorCollection();
    static void MinorCollection();
  };
}

#endif //POSEIDON_SCAVENGER_H
