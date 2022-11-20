#ifndef POSEIDON_OBJECT_H
#define POSEIDON_OBJECT_H

#include "poseidon/type/type.h"
#include "poseidon/pointer.h"

namespace poseidon {
 class Object{
  protected:
   Object() = default;
  public:
   virtual ~Object() = default;
   virtual TypeId GetTypeId() const = 0;

   uword GetStartingAddress() const {
     return (uword)this;
   }

   Pointer* raw_ptr() const {
     return (Pointer*)(GetStartingAddress() - sizeof(Pointer));
   }
 };

#define DEFINE_OBJECT(Name) \
 public:                    \
  static const constexpr TypeId kTypeId = TypeId::k##Name##TypeId; \
  TypeId GetTypeId() const override { return kTypeId; }            \
  static Class* GetClass() { return kClass; }                      \
 private:                   \
  static Class* kClass;     \
  static Class* CreateClass();
}

#endif // POSEIDON_OBJECT_H