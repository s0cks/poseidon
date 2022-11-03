#ifndef POSEIDON_NULL_H
#define POSEIDON_NULL_H

#include "poseidon/type/instance.h"

namespace poseidon {
 class Null : public Instance {
  protected:
   Null();
  public:
   ~Null() override = default;
   static Null* Get();
   DEFINE_OBJECT(Null);
 };
}

#endif // POSEIDON_NULL_H