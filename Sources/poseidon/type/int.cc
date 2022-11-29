#include "poseidon/type/int.h"
#include "poseidon/type/class.h"

namespace poseidon {
 Class* Int::kClass = nullptr;
 Field* Int::kValueField = nullptr;

 Class* Int::CreateClass() {
   LOG_IF(FATAL, kClass != nullptr) << kClassName << " class is already initialized";
   auto cls = kClass = new Class(kClassName, kTypeId, Class::kNumberClass);
   kValueField = cls->CreateField("value", cls);
   return cls;
 }
}