#include "poseidon/type/number.h"

namespace poseidon {
 Class* Number::kClass = nullptr;
 Field* Number::kValueField = nullptr;

 void* Number::operator new(size_t) noexcept {
   if(kClass == nullptr)
     LOG(FATAL) << "Number class not initialized";
   return malloc(kClass->GetAllocationSize()); //TODO: change to heap
 }

 void Number::operator delete(void* ptr) noexcept {
   // do nothing?
   free(ptr); //TODO: change to heap
 }

 Class* Number::CreateClass() {
   auto cls = kClass = new Class("Number", kTypeId);
   kValueField = cls->CreateField("value", cls);
   return cls;
 }

 Number::Number(const uword value):
  Instance(kClass, kTypeId) {
   Set(value);
 }

 uword Number::Get() const {
   return *((uword*) (((uword) this) + kValueField->GetOffset()));
 }

 void Number::Set(const uword value) {
   *((uword*) (((uword) this) + kValueField->GetOffset())) = value;
 }
}