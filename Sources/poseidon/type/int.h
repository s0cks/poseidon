#ifndef POSEIDON_INT_H
#define POSEIDON_INT_H

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

   uword GetStartingAddress() const {
     return (uword)this;
   }
  public:
   ~Int() override = default;

   RawObject* raw_ptr() const {
     return (RawObject*)(GetStartingAddress() - sizeof(RawObject));
   }

   RawInt Get() const;

   friend std::ostream& operator<<(std::ostream& stream, const Int& value) {
     stream << "Int(";
     stream << "value=" << +value.Get();
     stream << ")";
     return stream;
   }

   DEFINE_OBJECT(Int);
  public:
   void* operator new(size_t) noexcept;
   void operator delete(void*) noexcept;

   static inline Int* New(const RawInt value) {
     return new Int(value);
   }

   static inline Int* New() {
     return New(0);
   }
 };
}

#endif // POSEIDON_INT_H