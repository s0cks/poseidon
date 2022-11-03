#ifndef POSEIDON_SHORT_H
#define POSEIDON_SHORT_H

#include "poseidon/type/instance.h"

namespace poseidon {
 typedef uint16_t RawShort;

 class Short : public Instance {
   friend class Class;
   friend class Instance;
  protected:
   static Field* kValueField;

   explicit Short(RawShort value);
   void Set(RawShort value);
  public:
   ~Short() override = default;
   RawShort Get() const;

   friend std::ostream& operator<<(std::ostream& stream, const Short& value) {
     stream << "Short(";
     stream << "value=" << +value.Get();
     stream << ")";
     return stream;
   }

   DEFINE_OBJECT(Short);
  public:
   void* operator new(size_t) noexcept;
   void operator delete(void*) noexcept;

   static inline Short* New(const RawShort value) {
     return new Short(value);
   }

   static inline Short* New() {
     return New(0);
   }
 };
}

#endif // POSEIDON_SHORT_H