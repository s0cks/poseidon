#include "poseidon/type/null.h"
#include "poseidon/type/class.h"

namespace poseidon {
 Class* Null::kClass = nullptr;

 Class* Null::CreateClass() {
   return kClass = new Class("Null", kTypeId);
 }

 Null::Null():
  Instance(kClass, kTypeId) {
 }

 static Null* kNull = nullptr;
 Null* Null::Get() {
   if(kClass == nullptr)
     return nullptr;
   if(kNull == nullptr)
     kNull = Instance::New<Null>(kClass);
   return kNull;
 }
}