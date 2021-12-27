#include "raw_object.h"
#include "object.h"

namespace poseidon{
 const uint8_t RawObject::kMagic = 0xCA;

  void RawObject::VisitPointers(RawObjectPointerVisitor* vis) const{
    return GetObjectPointer()->VisitPointers(vis);
  }

  void RawObject::VisitPointers(RawObjectPointerPointerVisitor* vis) const{
    return GetObjectPointer()->VisitPointers(vis);
  }
}