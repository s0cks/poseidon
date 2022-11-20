#include "poseidon/type/tuple.h"
#include "poseidon/heap/heap.h"

namespace poseidon {
 Class* Tuple::kClass = nullptr;
 Field* Tuple::kCarField = nullptr;
 Field* Tuple::kCdrField = nullptr;

 void* Tuple::operator new(const size_t) noexcept {
  LOG_IF(FATAL, kClass == nullptr) << "Tuple class not initialized";
  auto heap = Heap::GetCurrentThreadHeap();
  auto address = heap->TryAllocate(kClass->GetAllocationSize());
  LOG_IF(FATAL, address == UNALLOCATED) << "cannot allocate Tuple";
  return ((Pointer*)address)->GetPointer();
 }

 void Tuple::operator delete(void*) noexcept {
   // do nothing
 }

 Class* Tuple::CreateClass() {
   auto cls = kClass = new Class("Tuple", kTypeId);
   kCarField = cls->CreateField("car", Class::kObjectClass);
   kCdrField = cls->CreateField("cdr", Class::kObjectClass);
   return cls;
 }
}