#ifndef POSEIDON_NUMBER_H
#define POSEIDON_NUMBER_H

#include "poseidon/type/instance.h"

namespace poseidon {
 class Number : public Instance {
   friend class Class;
   friend class Instance;
  protected:
   static Field* kValueField;

   explicit Number(uword value);
   void Set(uword value);
  public:
   ~Number() override = default;

   uword Get() const;

   friend std::ostream& operator<<(std::ostream& stream, const Number& value) {
     stream << "Number(";
     stream << "value=" << +value.Get() << ")";
     stream << ")";
     return stream;
   }
   DEFINE_OBJECT(Number);
  public:
   void* operator new(size_t) noexcept;
   void operator delete(void*) noexcept;

   static inline Number* New(const uword value) {
     return new Number(value);
   }

   static inline Number* New() {
     return New(0);
   }
 };
}

#endif // POSEIDON_NUMBER_H