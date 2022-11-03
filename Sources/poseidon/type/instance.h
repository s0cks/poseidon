#ifndef POSEIDON_INSTANCE_H
#define POSEIDON_INSTANCE_H

#include "poseidon/raw_object.h"
#include "poseidon/type/class.h"
#include "poseidon/type/field.h"

namespace poseidon {
 class Instance : public Object {
  protected:
   Class* type_;
   TypeId type_id_;

   explicit Instance(Class* type, const TypeId type_id = TypeId::kUnknownTypeId):
     Object(),
     type_(type),
     type_id_(type_id) {
   }

   uword FieldAddrAtOffset(int64_t offset) const {
     return ((uword)this) + offset;
   }

   Instance** FieldAddr(const Field* field) const {
     return (Instance**)FieldAddrAtOffset(field->GetOffset());
   }
  public:
   ~Instance() override = default;

   TypeId GetTypeId() const override {
     return type_id_;
   }

   Class* GetType() const {
     return type_;
   }

   Instance* GetField(const Field* field) const {
     return *FieldAddr(field);
   }

   void SetField(const Field* field, Instance* value) {
     *FieldAddr(field) = value;
   }
  public:
   template<class T>
   static T* New(Class* cls);
 };
}

#endif // POSEIDON_INSTANCE_H