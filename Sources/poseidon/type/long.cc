#include "poseidon/heap/heap.h"
#include "poseidon/type/long.h"
#include "poseidon/type/class.h"

namespace poseidon {
 Class* Long::kClass = nullptr;
 Field* Long::kValueField = nullptr;

 Class* Long::CreateClass() {
   LOG_IF(FATAL, kClass != nullptr) << kClassName << " class is already initialized";
   auto cls = kClass = new Class(kClassName, kTypeId, Class::kNumberClass);
   kValueField = cls->CreateField("value", cls);
   return cls;
 }
}