#ifndef POSEIDON_LONG_H
#define POSEIDON_LONG_H

#include "poseidon/type/instance.h"

namespace poseidon {
 typedef uint64_t RawLong;

 class Long : public Instance {
   friend class Class;
   friend class Instance;
  protected:
   static Field* kValueField;

   explicit Long(RawLong value);
   void Set(RawLong value);
  public:
   ~Long() override = default;

   RawLong Get() const;

   friend std::ostream& operator<<(std::ostream& stream, const Long& value) {
     stream << "Byte(";
     stream << "value=" << +value.Get();
     stream << ")";
     return stream;
   }

  DEFINE_OBJECT(Long);
  public:
   void* operator new(size_t) noexcept;
   void operator delete(void*) noexcept;

   static inline Long* New(const RawLong value) {
     return new Long(value);
   }

   static inline Long* New() {
     return New(0);
   }
 };
}

#endif // POSEIDON_LONG_H