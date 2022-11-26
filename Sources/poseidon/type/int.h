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

   explicit Int(RawInt value);
   void Set(RawInt value);
  public:
   ~Int() override = default;

   RawInt Get() const;

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
   template<class Z>
   void* operator new(size_t, Z* zone) noexcept {
     if(kClass == nullptr)
       LOG(FATAL) << "Int class not initialized";
     auto address = zone->TryAllocateBytes(kClass->GetAllocationSize());
     if(address == UNALLOCATED)
       LOG(FATAL) << "cannot allocate Int";
     return (void*)address;
   }

   void* operator new(size_t) noexcept;
   void operator delete(void*) noexcept;

   static inline int
   Compare(const Int& lhs, const Int& rhs) {
     if(lhs.Get() < rhs.Get())
       return -1;
     else if(lhs.Get() > rhs.Get())
       return +1;
     PSDN_ASSERT(lhs.Get() == rhs.Get());
     return 0;
   }

   static inline Int* New(const RawInt value) {
     return new Int(value);
   }

   static inline Int* New() {
     return New(0);
   }

   template<class Z>
   static inline Int* TryAllocateIn(Z* zone, const RawInt value = 0) {
     return new (zone)Int(value);
   }
 };
}

#endif // POSEIDON_INT_H