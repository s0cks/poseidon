#ifndef POSEIDON_NULL_H
#define POSEIDON_NULL_H

#include "poseidon/heap/heap.h"
#include "poseidon/type/instance.h"

namespace poseidon {
 class Null : public Instance {
  protected:
   Null():
    Instance(kClass, kTypeId) {
   }
  public:
   ~Null() override = default;
   DEFINE_OBJECT(Null);

   friend std::ostream& operator<<(std::ostream& stream, const Null& value) {
     return stream << "Null()";
   }
  public:
   void* operator new(size_t) noexcept {
     CHECK_CLASS_IS_INITIALIZED(FATAL);
     LOG_IF(FATAL, !Heap::CurrentThreadHasHeap()) << "current thread `" << GetCurrentThreadName() << "` does not have a heap";
     auto heap = Heap::GetCurrentThreadHeap();
     auto address = heap->TryAllocateBytes(kClass->GetAllocationSize());
     LOG_IF(FATAL, address == UNALLOCATED) << "cannot allocate new" << kClassName;
     return (void*) address;
   }

   template<class Z>
   void* operator new(size_t, Z* zone) noexcept {
     CHECK_CLASS_IS_INITIALIZED(FATAL);
     LOG_IF(FATAL, !Heap::CurrentThreadHasHeap()) << "current thread `" << GetCurrentThreadName() << "` does not have a heap";
     auto address = zone->TryAllocateBytes(kClass->GetAllocationSize());
     LOG_IF(FATAL, address == UNALLOCATED) << "cannot allocate new" << kClassName;
     return (void*) address;
   }

   void operator delete(void*) noexcept { /* do nothing */ }

   template<class Z>
   static inline Null* TryAllocateIn(Z* zone) {
     return new (zone)Null();
   }

   static inline Null* New() {
     return new Null();
   }

   static Null* Get();
 };
}

#endif // POSEIDON_NULL_H