#include "raw_object.h"
#include "object.h"

namespace poseidon{
  void RawObject::VisitPointers(RawObjectPointerVisitor* vis){
    return GetObjectPointer()->VisitPointers(vis);
  }

  void RawObject::VisitPointers(RawObjectPointerPointerVisitor* vis){
    return GetObjectPointer()->VisitPointers(vis);
  }
}