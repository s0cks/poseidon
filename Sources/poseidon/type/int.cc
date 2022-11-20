#include "poseidon/type/int.h"
#include "poseidon/heap/heap.h"
#include "poseidon/type/class.h"

namespace poseidon {
 Class* Int::kClass = nullptr;
 Field* Int::kValueField = nullptr;

 void* Int::operator new(const size_t) noexcept {
   if(kClass == nullptr)
     LOG(FATAL) << "Int class not initialized";
   auto heap = Heap::GetCurrentThreadHeap();
   auto address = heap->TryAllocate(kClass->GetAllocationSize());
   if(address == UNALLOCATED)
     LOG(FATAL) << "cannot allocate Int";
   return ((Pointer*)address)->GetPointer();
 }

 void Int::operator delete(void* ptr) noexcept {
   // do nothing
 }

 Class* Int::CreateClass() {
   auto cls = kClass = new Class("Int", kTypeId, Class::kNumberClass);
   kValueField = cls->CreateField("value", cls);
   return cls;
 }

 Int::Int(const RawInt value):
   Instance(kClass, kTypeId) {
   Set(value);
 }

 RawInt Int::Get() const {
   return *((RawInt*) FieldAddrAtOffset(kValueField->GetOffset()));
 }

 void Int::Set(const RawInt value) {
   *((RawInt*) FieldAddrAtOffset(kValueField->GetOffset())) = value;
 }
}