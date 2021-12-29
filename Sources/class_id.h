#ifndef POSEIDON_CLASS_ID_H
#define POSEIDON_CLASS_ID_H

#include <string>
#include <ostream>
#include "platform.h"

namespace poseidon{
#define FOR_EACH_CLASS(V) \
 V(Bool)                  \
 V(Byte)                  \
 V(Short)                 \
 V(Int)                   \
 V(Long)                  \
 V(String)                \
 V(Class)                 \
 V(Field)                 \
 V(Array)

 enum ClassId : intptr_t{
   kIllegalCid = 0,

#define DEFINE_CLASS_ID(Name) k##Name##Cid,
  FOR_EACH_CLASS(DEFINE_CLASS_ID)
#undef DEFINE_CLASS_ID

    kNumberOfPredefinedCids,
 };

  static inline std::ostream&
  operator<<(std::ostream& stream, const ClassId& cls_id){
    switch(cls_id){
#define DEFINE_TOSTRING(Name) \
      case ClassId::k##Name##Cid: return stream << #Name;
      FOR_EACH_CLASS(DEFINE_TOSTRING)
#undef DEFINE_TOSTRING
      case ClassId::kIllegalCid:
      default:
        return stream << "IllegalCid";
    }
  }
}

#endif //POSEIDON_CLASS_ID_H
