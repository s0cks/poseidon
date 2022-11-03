#include "poseidon/type/byte.h"
#include "poseidon/heap/heap.h"
#include "poseidon/type/class.h"

namespace poseidon {
 Class* Byte::kClass = nullptr;
 Field* Byte::kValueField = nullptr;

#ifndef UNALLOCATED
#define UNALLOCATED 0 //TODO: cleanup
#endif // UNALLOCATED

 void* Byte::operator new(const size_t) noexcept {
   if(kClass == nullptr)
     LOG(FATAL) << "Byte class not initialized";
   auto heap = Heap::GetCurrentThreadHeap();
   auto address = heap->TryAllocate(kClass->GetAllocationSize());
   if(address == UNALLOCATED)
     LOG(FATAL) << "cannot allocate Byte";
   return ((RawObject*)address)->GetPointer();
 }

 void Byte::operator delete(void* ptr) noexcept {
   // do nothing
 }

 Class* Byte::CreateClass() {
   auto cls = kClass = new Class("Byte", kTypeId, Class::kNumberClass);
   kValueField = cls->CreateField("value", cls);
   return cls;
 }

 Byte::Byte(const RawByte value):
  Instance(kClass, kTypeId) {
   Set(value);
 }

 RawByte Byte::Get() const {
   return *((RawByte*) FieldAddrAtOffset(kValueField->GetOffset()));
 }

 void Byte::Set(const RawByte value) {
   *((RawByte*) FieldAddrAtOffset(kValueField->GetOffset())) = value;
 }
}