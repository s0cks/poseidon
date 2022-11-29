#include "poseidon/type/byte.h"
#include "poseidon/heap/heap.h"
#include "poseidon/type/class.h"

namespace poseidon {
 Class* Byte::kClass = nullptr;
 Field* Byte::kValueField = nullptr;

 Class* Byte::CreateClass() {
   LOG_IF(FATAL, kClass != nullptr) << kClassName << " class is already initialized";
   auto cls = kClass = new Class(kClassName, kTypeId, Class::kNumberClass);
   kValueField = cls->CreateField("value", cls);
   return cls;
 }
}