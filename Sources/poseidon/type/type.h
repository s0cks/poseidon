#ifndef POSEIDON_TYPE_H
#define POSEIDON_TYPE_H

#include "poseidon/platform/platform.h"

namespace poseidon {
#define FOR_EACH_TYPE(V) \
 V(Object)               \
 V(Null)                 \
 V(Bool)                 \
 V(Byte)                 \
 V(Short)                \
 V(Int)                  \
 V(Long)                 \
 V(Class)                \
 V(Field)                \
 V(Number)               \
 V(Tuple)

#define DEFINE_TYPE_ID(Name) k##Name##TypeId,
 enum class TypeId : word {
   kUnknownTypeId = 0,
   FOR_EACH_TYPE(DEFINE_TYPE_ID)
   kMaxTypeId,
 };
#undef DEFINE_TYPE_ID

#define FORWARD_DECLARE(Name) class Name;
  FOR_EACH_TYPE(FORWARD_DECLARE)
#undef FORWARD_DECLARE
}

#endif // POSEIDON_TYPE_H