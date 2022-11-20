#ifndef POSEIDON_CLASS_H
#define POSEIDON_CLASS_H

#include <string>
#include <glog/logging.h>
#include "poseidon/type/object.h"

namespace poseidon {
 class Class : public Object {
   friend class Instance;
  private:
   std::string name_;
   TypeId type_id_;
   Class* parent_;
   std::vector<Field*> fields_;
  public:
   explicit Class(std::string name, const TypeId type_id, Class* parent = kObjectClass):
     name_(std::move(name)),
     parent_(parent),
     fields_(),
     type_id_(type_id) {
   }
   ~Class() override = default;

   std::string GetName() const {
     return name_;
   }

   Class* GetParent() const {
     return parent_;
   }

   size_t GetFieldCount() const {
     return fields_.size();
   }

   Field* GetFieldAt(const size_t index) const {
     return fields_[index];
   }

   int64_t GetAllocationSize() const;
   Field* CreateField(std::string name, Class* type);

   friend std::ostream& operator<<(std::ostream& stream, const Class& value) {
     stream << "Class(";
     stream << "name=" << value.GetName() << ", ";
     if(value.GetParent() != nullptr)
       stream << "parent=" << value.GetParent()->GetName() << ", ";
     stream << "alloc-size=" << value.GetAllocationSize();
     stream << ")";
     return stream;
   }

    DEFINE_OBJECT(Class);
  public:
   static Class* kObjectClass;
   static Class* kNullClass;
   static Class* kBoolClass;
   static Class* kByteClass;
   static Class* kShortClass;
   static Class* kIntClass;
   static Class* kLongClass;
   static Class* kNumberClass;
   static Class* kTupleClass;

   static void Initialize();
 };
}

#endif // POSEIDON_CLASS_H