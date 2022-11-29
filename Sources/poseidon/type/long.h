#ifndef POSEIDON_LONG_H
#define POSEIDON_LONG_H

#include "poseidon/heap/heap.h"
#include "poseidon/type/instance.h"

namespace poseidon {
 typedef uint64_t RawLong;

 class Long : public Instance {
   friend class Class;
   friend class Instance;
  protected:
   static Field* kValueField;

   explicit Long(RawLong value):
    Instance(kClass, kTypeId) {
     Set(value);
   }
  public:
   ~Long() override = default;

   RawLong Get() const {
     CHECK_CLASS_IS_INITIALIZED(FATAL);
     return *((RawLong*) FieldAddrAtOffset(kValueField->GetOffset()));
   }

   void Set(const RawLong value) {
     CHECK_CLASS_IS_INITIALIZED(FATAL);
     *((RawLong*) FieldAddrAtOffset(kValueField->GetOffset())) = value;
   }

   friend std::ostream& operator<<(std::ostream& stream, const Long& value) {
     stream << "Long(";
     stream << "value=" << +value.Get();
     stream << ")";
     return stream;
   }

  DEFINE_OBJECT(Long);
  public:
   void* operator new(const size_t) noexcept {
     CHECK_CLASS_IS_INITIALIZED(FATAL);
     LOG_IF(FATAL, !Heap::CurrentThreadHasHeap()) << "current thread `" << GetCurrentThreadName() << "` does not have a heap";
     auto heap = Heap::GetCurrentThreadHeap();
     auto address = heap->TryAllocateBytes(kClass->GetAllocationSize());
     LOG_IF(FATAL, address == UNALLOCATED) << "cannot allocate new " << kClassName;
     return (void*)address;
   }

   template<class Z>
   void* operator new(const size_t, Z* zone) noexcept {
     CHECK_CLASS_IS_INITIALIZED(FATAL);
     auto address = zone->TryAllocateBytes(kClass->GetAllocationSize());
     LOG_IF(FATAL, address == UNALLOCATED) << "cannot allocate new " << kClassName;
     return (void*)address;
   }

   void operator delete(void*) noexcept { /* do nothing */ }

   template<class Z>
   static inline Long*
   TryAllocateIn(Z* zone, const RawLong value = 0) {
     return new (zone)Long(value);
   }

   static inline Long*
   New(const RawLong value = 0) {
     return new Long(value);
   }
 };
}

#endif // POSEIDON_LONG_H