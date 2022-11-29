#ifndef POSEIDON_BOOL_H
#define POSEIDON_BOOL_H

#include "poseidon/heap/heap.h"
#include "poseidon/type/instance.h"

namespace poseidon {
 typedef bool RawBool;

 class Bool : public Instance {
   friend class Class;
   friend class Instance;
  protected:
   static Field* kValueField;
  protected:
   explicit Bool(const RawBool value):
    Instance(kClass, kTypeId) {
     Set(value);
   }
  public:
   ~Bool() override = default;

   void Set(const RawBool value) { //TODO: visible for testing
     CHECK_CLASS_IS_INITIALIZED(FATAL);
     *((RawBool*) FieldAddrAtOffset(kValueField->GetOffset())) = value;
   }

   RawBool Get() const {
     CHECK_CLASS_IS_INITIALIZED(FATAL);
     return *((RawBool*) FieldAddrAtOffset(kValueField->GetOffset()));
   }

   friend std::ostream& operator<<(std::ostream& stream, const Bool& value) {
     stream << "Bool(";
     stream << "value=" << (value.Get() ? "true" : "false");
     stream << ")";
     return stream;
   }

   DEFINE_OBJECT(Bool);
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

   template<class Z>
   static inline Bool*
   TryAllocateIn(Z* zone, const bool value) {
     return new (zone)Bool(value);
   }

   static inline Bool*
   New(const bool value) {
     return new Bool(value);
   }

   static Bool* True();
   static Bool* False();
 };
}

#endif // POSEIDON_BOOL_H