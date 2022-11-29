#ifndef POSEIDON_BYTE_H
#define POSEIDON_BYTE_H

#include "poseidon/heap/heap.h"
#include "poseidon/type/number.h"

namespace poseidon {
 typedef uint8_t RawByte;

 class Byte : public Instance {
   friend class Class;
   friend class Instance;
  protected:
   static Field* kValueField;
  protected:
   explicit Byte(RawByte value):
    Instance(kClass, kTypeId) {
     Set(value);
   }
  public:
   ~Byte() override = default;

   RawByte Get() const {
     CHECK_CLASS_IS_INITIALIZED(FATAL);
     return *((RawByte*) FieldAddrAtOffset(kValueField->GetOffset()));
   }

   void Set(const RawByte value) {
     CHECK_CLASS_IS_INITIALIZED(FATAL);
     *((RawByte*) FieldAddrAtOffset(kValueField->GetOffset())) = value;
   }

   friend std::ostream& operator<<(std::ostream& stream, const Byte& value) {
     stream << "Byte(";
     stream << "value=" << +value.Get();
     stream << ")";
     return stream;
   }

   DEFINE_OBJECT(Byte);
  public:
   void* operator new(size_t) noexcept {
     CHECK_CLASS_IS_INITIALIZED(FATAL);
     LOG_IF(FATAL, !Heap::CurrentThreadHasHeap()) << "current thread `" << GetCurrentThreadName() << "` does not have a heap";
     auto heap = Heap::GetCurrentThreadHeap();
     auto address = heap->TryAllocateClassBytes(kClass);
     LOG_IF(FATAL, address == UNALLOCATED) << "cannot allocate new " << kClassName;
     return (void*) address;
   }

   template<class Z>
   void* operator new(size_t, Z* zone) noexcept {
     CHECK_CLASS_IS_INITIALIZED(FATAL);
     auto address = zone->TryAllocateClassBytes(kClass);
     LOG_IF(FATAL, address == UNALLOCATED) << "cannot allocate new " << kClassName;
     return (void*) address;
   }

   void operator delete(void*) noexcept { /* do nothing */ }

   static inline Byte* New(const RawByte value = 0) {
     return new Byte(value);
   }

   template<class Z>
   static inline Byte* TryAllocateIn(Z* zone, const RawByte value = 0) {
     return new (zone)Byte(value);
   }
 };
}

#endif // POSEIDON_BYTE_H