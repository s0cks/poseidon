#include "poseidon/heap/heap.h"
#include "poseidon/type/long.h"
#include "poseidon/type/class.h"

namespace poseidon {
 Class* Long::kClass = nullptr;
 Field* Long::kValueField = nullptr;

#ifndef UNALLOCATED
#define UNALLOCATED 0 // TODO: cleanup
#endif // UNALLOCATED

 void* Long::operator new(const size_t) noexcept {
   if(kClass == nullptr)
     LOG(FATAL) << "Long class not initialized";
   auto heap = Heap::GetCurrentThreadHeap();
   auto address = heap->TryAllocate(kClass->GetAllocationSize());
   if(address == UNALLOCATED)
     LOG(FATAL) << "cannot allocate Long";
   return ((Pointer*)address)->GetPointer();
 }

 void Long::operator delete(void* ptr) noexcept {
   // do nothing
 }

 Class* Long::CreateClass() {
   auto cls = kClass = new Class("Long", kTypeId, Class::kNumberClass);
   kValueField = cls->CreateField("value", cls);
   return cls;
 }

 Long::Long(const RawLong value):
     Instance(kClass, kTypeId) {
   Set(value);
 }

 RawLong Long::Get() const {
   return *((RawLong*) FieldAddrAtOffset(kValueField->GetOffset()));
 }

 void Long::Set(const RawLong value) {
   *((RawLong*) FieldAddrAtOffset(kValueField->GetOffset())) = value;
 }
}