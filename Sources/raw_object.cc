#include "raw_object.h"
#include "object.h"

namespace poseidon{
  void RawObject::VisitPointers(RawObjectPointerVisitor* vis){
    if(IsRawType())
      return;//TODO: remove
    return GetObjectPointer()->VisitPointers(vis);
  }

  void RawObject::VisitPointers(RawObjectPointerPointerVisitor* vis){
    if(IsRawType())
      return;//TODO: remove
    return GetObjectPointer()->VisitPointers(vis);
  }
}