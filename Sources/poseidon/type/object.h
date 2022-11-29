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
  static constexpr const char* kClassName = #Name; \
  static const constexpr TypeId kTypeId = TypeId::k##Name##TypeId; \
  TypeId GetTypeId() const override { return kTypeId; }            \
  static Class* GetClass() { return kClass; }                      \
 private:                   \
  static Class* kClass;     \
  static Class* CreateClass();

#define CHECK_CLASS_IS_INITIALIZED(Severity) \
  LOG_IF(FATAL, kClass == nullptr) << "Class " << kClassName << " is not initialized";
}

#endif // POSEIDON_OBJECT_H