#ifndef POSEIDON_BOOL_H
#define POSEIDON_BOOL_H

#include "poseidon/type/instance.h"

namespace poseidon {
 class Bool : public Instance {
   friend class Class;
   friend class Instance;
  protected:
   explicit Bool(const bool value):
    Instance(kClass, kTypeId) {
     Set(value);
   }

   void Set(bool value);
  public:
   ~Bool() override = default;
   bool Get() const;

   friend std::ostream& operator<<(std::ostream& stream, const Bool& value) {
     stream << "Bool(";
     stream << "value=" << (value.Get() ? "true" : "false");
     stream << ")";
     return stream;
   }

   static Bool* True();
   static Bool* False();
   DEFINE_OBJECT(Bool);
   static Field* kValueField;
  public:
   void* operator new(size_t) noexcept;
   void operator delete(void*) noexcept;
 };
}

#endif // POSEIDON_BOOL_H