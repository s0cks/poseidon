#ifndef POSEIDON_INT_H
#define POSEIDON_INT_H

#include "poseidon/heap/heap.h"
#include "poseidon/type/instance.h"

namespace poseidon {
 typedef uint32_t RawInt;

 class Int : public Instance {
   friend class Class;
   friend class Instance;
  protected:
   static Field* kValueField;

   explicit Int(RawInt value):
    Instance(kClass, kTypeId) {
     Set(value);
   }
  public:
   ~Int() override = default;

   void Set(const RawInt value) {
     CHECK_CLASS_IS_INITIALIZED(FATAL);
     *((RawInt*) FieldAddrAtOffset(kValueField->GetOffset())) = value;
   }

   RawInt Get() const {
     CHECK_CLASS_IS_INITIALIZED(FATAL);
     return *((RawInt*) FieldAddrAtOffset(kValueField->GetOffset()));
   }

   friend std::ostream& operator<<(std::ostream& stream, const Int& value) {
     stream << "Int(";
     stream << "value=" << +value.Get();
     stream << ")";
     return stream;
   }

   friend bool operator==(const Int& lhs, const Int& rhs) {
     return Compare(lhs, rhs) == 0;
   }

   friend bool operator!=(const Int& lhs, const Int& rhs) {
     return Compare(lhs, rhs) != 0;
   }

   friend bool operator<(const Int& lhs, const Int& rhs) {
     return Compare(lhs, rhs) < 0;
   }

   friend bool operator>(const Int& lhs, const Int& rhs) {
     return Compare(lhs, rhs) > 0;
   }

   DEFINE_OBJECT(Int);
  public:
   void* operator new(size_t) noexcept {
     CHECK_CLASS_IS_INITIALIZED(FATAL);
     LOG_IF(FATAL, Heap::CurrentThreadHasHeap()) << "current thread `" << GetCurrentThreadName() << "` does not have a heap";
     auto heap = Heap::GetCurrentThreadHeap();
     auto address = heap->TryAllocateBytes(kClass->GetAllocationSize());
     LOG_IF(FATAL, address == UNALLOCATED) << "cannot allocate new " << kClassName;
     return (void*)address;
   }

   template<class Z>
   void* operator new(size_t, Z* zone) noexcept {
     CHECK_CLASS_IS_INITIALIZED(FATAL);
     auto address = zone->TryAllocateBytes(kClass->GetAllocationSize());
     LOG_IF(FATAL, address == UNALLOCATED) << "cannot allocate new " << kClassName;
     return (void*)address;
   }

   void operator delete(void*) noexcept { /* do nothing */ }

   static inline int
   Compare(const Int& lhs, const Int& rhs) {
     if(lhs.Get() < rhs.Get())
       return -1;
     else if(lhs.Get() > rhs.Get())
       return +1;
     PSDN_ASSERT(lhs.Get() == rhs.Get());
     return 0;
   }

   template<class Z>
   static inline Int* TryAllocateIn(Z* zone, const RawInt value = 0) {
     return new (zone)Int(value);
   }

   static inline Int* New(const RawInt value = 0) {
     return new Int(value);
   }
 };
}

#endif // POSEIDON_INT_H