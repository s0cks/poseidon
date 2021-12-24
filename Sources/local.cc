#include "local.h"
#include "raw_object.h"

namespace poseidon{
  RawObject* LocalBase::GetRawObjectPointer() const{
    return (*value_) ? (*value_) : nullptr;
  }

  Object* LocalBase::GetObjectPointer() const{
    return (*value_) ? (*value_)->GetObjectPointer() : nullptr;
  }
}