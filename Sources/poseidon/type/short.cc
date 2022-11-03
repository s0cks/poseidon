#include "poseidon/heap/heap.h"
#include "poseidon/type/short.h"
#include "poseidon/type/class.h"

namespace poseidon {
 Class* Short::kClass = nullptr;
 Field* Short::kValueField = nullptr;

#ifndef UNALLOCATED
#define UNALLOCATED 0 //TODO: cleanup
#endif // UNALLOCATED

 void* Short::operator new(const size_t) noexcept {
   if(kClass == nullptr)
     LOG(FATAL) << "Short class not initialized";
   auto heap = Heap::GetCurrentThreadHeap();
   auto address = heap->TryAllocate(kClass->GetAllocationSize());
   if(address == UNALLOCATED)
     LOG(FATAL) << "cannot allocate Short";
   return ((RawObject*)address)->GetPointer();
 }

 void Short::operator delete(void* ptr) noexcept {
   // do nothing
 }

 Class* Short::CreateClass() {
   auto cls = kClass = new Class("Short", kTypeId, Class::kNumberClass);
   kValueField = cls->CreateField("value", cls);
   return cls;
 }

 Short::Short(const RawShort value):
  Instance(kClass, kTypeId) {
   Set(value);
 }

 RawShort Short::Get() const {
   return *((RawShort*) FieldAddrAtOffset(kValueField->GetOffset()));
 }

 void Short::Set(const RawShort value) {
   *((RawShort*) FieldAddrAtOffset(kValueField->GetOffset())) = value;
 }
}