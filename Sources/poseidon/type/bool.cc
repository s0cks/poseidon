#include "poseidon/type/bool.h"
#include "poseidon/heap/heap.h"

namespace poseidon {
 Class* Bool::kClass = nullptr;
 Field* Bool::kValueField = nullptr;

#ifndef UNALLOCATED
#define UNALLOCATED 0 //TODO: cleanup
#endif // UNALLOCATED

 void* Bool::operator new(size_t sz) noexcept {
   if(kClass == nullptr)
     LOG(FATAL) << "Bool class not initialized";
   auto heap = Heap::GetCurrentThreadHeap();
   auto ptr = heap->TryAllocate(kClass->GetAllocationSize());
   if(ptr == UNALLOCATED)
     LOG(FATAL) << "cannot allocate Bool";
   return ((Pointer*)ptr)->GetPointer();
 }

 void Bool::operator delete(void* ptr) noexcept {
   // do nothing
 }

 Class* Bool::CreateClass() {
   auto cls = kClass = new Class("Bool", kTypeId);
   kValueField = cls->CreateField("value", cls);
   return cls;
 }

 bool Bool::Get() const {
   return *((bool*) (((uword) this) + kValueField->GetOffset()));
 }

 void Bool::Set(const bool value) {
   *((bool*) (((uword) this) + kValueField->GetOffset())) = value;
 }

 static Bool* kTrue = nullptr;
 Bool* Bool::True() {
   if(Class::kBoolClass == nullptr || kValueField == nullptr)
     return nullptr;
   if(kTrue == nullptr)
     kTrue = new Bool(true);
   return kTrue;
 }

 static Bool* kFalse = nullptr;
 Bool* Bool::False() {
   if(Class::kBoolClass == nullptr || kValueField == nullptr)
     return nullptr;
   if(kFalse == nullptr)
     kFalse = new Bool(false);
   return kFalse;
 }
}