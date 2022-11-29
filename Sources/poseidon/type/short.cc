#include "poseidon/type/short.h"
#include "poseidon/type/class.h"

namespace poseidon {
 Class* Short::kClass = nullptr;
 Field* Short::kValueField = nullptr;

 Class* Short::CreateClass() {
   LOG_IF(FATAL, kClass != nullptr) << kClassName << " is already initialized";
   auto cls = kClass = new Class(kClassName, kTypeId, Class::kNumberClass);
   kValueField = cls->CreateField("value", cls);
   return cls;
 }
}