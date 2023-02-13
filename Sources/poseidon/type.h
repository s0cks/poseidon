#ifndef PSDN_TYPE_H
#define PSDN_TYPE_H

#include <glog/logging.h>
#include "poseidon/platform/platform.h"

namespace poseidon {
#define FOR_EACH_SIGNED_INT_TYPE(V) \
 V(Int8)                            \
 V(Int16)                           \
 V(Int32)                           \
 V(Int64)

#define FOR_EACH_UNSIGNED_INT_TYPE(V) \
 V(UInt8)                             \
 V(UInt16)                            \
 V(UInt32)                            \
 V(UInt64)

#define FOR_EACH_INT_TYPE(V) \
 FOR_EACH_SIGNED_INT_TYPE(V) \
 FOR_EACH_UNSIGNED_INT_TYPE(V)

#define FOR_EACH_TYPE(V) \
 V(Object)               \
 V(Null)                 \
 V(Bool)                 \
 V(Class)                \
 V(Field)                \
 V(Number)               \
 FOR_EACH_INT_TYPE(V)    \
 V(Tuple)

#define DEFINE_TYPE_ID(Name) k##Name##TypeId,
 enum class TypeId : word {
   kUnknownTypeId = 0,
   FOR_EACH_TYPE(DEFINE_TYPE_ID)
   kMaxTypeId,
 };
#undef DEFINE_TYPE_ID

 static std::ostream& operator<<(std::ostream& stream, const TypeId& type_id) {
   switch(type_id) {
#define DEFINE_TOSTRING(Name) \
     case TypeId::k##Name##TypeId: return stream << #Name;
     FOR_EACH_TYPE(DEFINE_TOSTRING);
     case TypeId::kMaxTypeId:
     case TypeId::kUnknownTypeId:
     default:
       return stream << "[unknown " << static_cast<word>(type_id) << "]";
   }
#undef DEFINE_TOSTRING
 }

 template<const google::LogSeverity Severity=google::INFO>
 static inline void
 PrintAllTypeIds() {
   LOG_AT_LEVEL(Severity) << "Type Ids:";
   for(auto idx = static_cast<word>(TypeId::kUnknownTypeId);
        idx <= static_cast<word>(TypeId::kMaxTypeId);
        idx++) {
     LOG_AT_LEVEL(Severity) << " - #" << idx << ": " << static_cast<TypeId>(idx);
   }
 }

#define FORWARD_DECLARE(Name) class Name;
 FOR_EACH_TYPE(FORWARD_DECLARE)
#undef FORWARD_DECLARE
}

#endif //PSDN_TYPE_H