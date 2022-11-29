#include "poseidon/type/null.h"
#include "poseidon/type/class.h"

namespace poseidon {
 Class* Null::kClass = nullptr;

 Class* Null::CreateClass() {
   LOG_IF(FATAL, kClass != nullptr) << kClassName << " class is already initialized";
   auto cls = kClass = new Class(kClassName, kTypeId);
   return cls;
 }

 static Null* kNull = nullptr;
 Null* Null::Get() {
   CHECK_CLASS_IS_INITIALIZED()
   if(kNull == nullptr)
     kNull = New();
   return kNull;
 }
}