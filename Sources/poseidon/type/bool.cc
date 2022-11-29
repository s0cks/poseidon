#include "poseidon/type/bool.h"
#include "poseidon/heap/heap.h"

namespace poseidon {
 Class* Bool::kClass = nullptr;
 Field* Bool::kValueField = nullptr;

 Class* Bool::CreateClass() {
   LOG_IF(FATAL, kClass != nullptr) << kClassName << " class is already initialized";
   auto cls = kClass = new Class(kClassName, kTypeId);
   kValueField = cls->CreateField("value", cls);
   return cls;
 }

 static Bool* kTrue = nullptr;
 Bool* Bool::True() {
   CHECK_CLASS_IS_INITIALIZED(FATAL);
   if(kTrue == nullptr)
     kTrue = New(true);
   return kTrue;
 }

 static Bool* kFalse = nullptr;
 Bool* Bool::False() {
   CHECK_CLASS_IS_INITIALIZED(FATAL);
   if(kFalse == nullptr)
     kFalse = New(false);
   return kFalse;
 }
}