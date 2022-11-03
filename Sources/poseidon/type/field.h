#ifndef POSEIDON_FIELD_H
#define POSEIDON_FIELD_H

#include <string>
#include "poseidon/type/object.h"

namespace poseidon {
 class Field : public Object {
   friend class Class;
  protected:
   std::string name_;
   Class* type_;
   Class* owner_;
   int64_t offset_;

   Field(std::string name, Class* type, Class* owner):
     name_(std::move(name)),
     type_(type),
     owner_(owner),
     offset_(0) {
   }

   void SetOffset(const int64_t offset) {
     offset_ = offset;
   }
  public:
   ~Field() override = default;

   std::string GetName() const {
     return name_;
   }

   Class* GetType() const {
     return type_;
   }

   Class* GetOwner() const {
     return owner_;
   }

   int64_t GetOffset() const {
     return offset_;
   }

   DEFINE_OBJECT(Field);
 };
}

#endif // POSEIDON_FIELD_H