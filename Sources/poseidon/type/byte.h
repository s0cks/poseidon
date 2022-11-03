#ifndef POSEIDON_BYTE_H
#define POSEIDON_BYTE_H

#include "poseidon/type/number.h"

namespace poseidon {
 typedef uint8_t RawByte;

 class Byte : public Instance {
   friend class Class;
   friend class Instance;
  protected:
   static Field* kValueField;

   explicit Byte(RawByte value);
   void Set(RawByte value);
  public:
   ~Byte() override = default;

   RawByte Get() const;

   friend std::ostream& operator<<(std::ostream& stream, const Byte& value) {
     stream << "Byte(";
     stream << "value=" << +value.Get();
     stream << ")";
     return stream;
   }

   DEFINE_OBJECT(Byte);
  public:
   void* operator new(size_t) noexcept;
   void operator delete(void*) noexcept;

   static inline Byte* New(const RawByte value) {
     return new Byte(value);
   }

   static inline Byte* New() {
     return New(0);
   }
 };
}

#endif // POSEIDON_BYTE_H