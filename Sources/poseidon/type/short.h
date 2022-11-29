#ifndef POSEIDON_SHORT_H
#define POSEIDON_SHORT_H

#include "poseidon/heap/heap.h"
#include "poseidon/type/instance.h"

namespace poseidon {
 typedef uint16_t RawShort;

 class Short : public Instance {
   friend class Class;
   friend class Instance;
  protected:
   static Field* kValueField;

   explicit Short(RawShort value):
    Instance(kClass, kTypeId) {
     Set(value);
   }
  public:
   ~Short() override = default;

   RawShort Get() const {
     LOG_IF(FATAL, kClass == nullptr) << "Short class is not initialized";
     return *((RawShort*) FieldAddrAtOffset(kValueField->GetOffset()));
   }

   void Set(const RawShort value) {
     LOG_IF(FATAL, kClass == nullptr) << "Short class is not initialized";
     *((RawShort*) FieldAddrAtOffset(kValueField->GetOffset())) = value;
   }

   friend std::ostream& operator<<(std::ostream& stream, const Short& value) {
     stream << "Short(";
     stream << "value=" << +value.Get();
     stream << ")";
     return stream;
   }

   DEFINE_OBJECT(Short);
  public:
   void* operator new(size_t sz) noexcept {
     LOG_IF(FATAL, kClass == nullptr) << "Short class is not initialized";
     LOG_IF(FATAL, !Heap::CurrentThreadHasHeap()) << "current thread `" << GetCurrentThreadName() << "` does not have a heap";
     auto heap = Heap::GetCurrentThreadHeap();
     auto address = heap->TryAllocateBytes(kClass->GetAllocationSize());
     LOG_IF(FATAL, address == UNALLOCATED) << "cannot allocate new " << kClassName;
     return (void*) address;
   }

   template<class Z>
   void* operator new(size_t, Z* zone) noexcept {
     LOG_IF(FATAL, kClass == nullptr) << "Short class is not initialized";
     auto address = zone->TryAllocateBytes(kClass->GetAllocationSize());
     LOG_IF(FATAL, address == UNALLOCATED) << "cannot allocate new " << kClassName;
     return (void*) address;
   }

   void operator delete(void*) noexcept { /* do nothing */ }

   template<class Z>
   static inline Short*
   TryAllocateIn(Z* zone, const RawShort value = 0) {
     return new (zone)Short(value);
   }

   static inline Short*
   New(const RawShort value = 0) {
     return new Short(value);
   }
 };
}

#endif // POSEIDON_SHORT_H