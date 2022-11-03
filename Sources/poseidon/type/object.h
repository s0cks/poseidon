#ifndef POSEIDON_OBJECT_H
#define POSEIDON_OBJECT_H

#include "poseidon/type/type.h"

namespace poseidon {
 class Object{
  protected:
   Object() = default;
  public:
   virtual ~Object() = default;
   virtual TypeId GetTypeId() const = 0;
 };

#define DEFINE_OBJECT(Name) \
 public:                    \
  static const constexpr TypeId kTypeId = TypeId::k##Name##TypeId; \
  TypeId GetTypeId() const override { return kTypeId; }            \
 private:                   \
  static Class* kClass;     \
  static Class* CreateClass();
}

#endif // POSEIDON_OBJECT_H