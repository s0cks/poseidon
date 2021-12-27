#include <glog/logging.h>
#include "object.h"
#include "allocator.h"

namespace poseidon{
  Class* Class::CLASS_OBJECT = nullptr;
  Class* Class::CLASS_CLASS = nullptr;
  Class* Class::CLASS_FIELD = nullptr;

  // numbers
  Class* Class::CLASS_NUMBER = nullptr;
  Field* Number::kValueField = nullptr;
  Class* Class::CLASS_BYTE = nullptr;
  Class* Class::CLASS_INT = nullptr;

  // bool
  Class* Class::CLASS_BOOL = nullptr;
  Field* Bool::kValueField = nullptr;

  // array
  Class* Class::CLASS_ARRAY = nullptr;
  Field* Array::kLengthField = nullptr;
  Field* Array::kDataField = nullptr;

  void Class::Initialize(){
    Class::CLASS_OBJECT = new Class("Object", nullptr);
    Class::CLASS_CLASS = new Class("Class", CLASS_OBJECT);
    Class::CLASS_FIELD = new Class("Field", CLASS_OBJECT);

    // numbers
    Class::CLASS_NUMBER = new Class("Number", CLASS_OBJECT);
    Number::kValueField = CLASS_NUMBER->CreateField("value", CLASS_NUMBER);
    Class::CLASS_BYTE = new Class("Byte", CLASS_NUMBER);
    Class::CLASS_INT = new Class("Int", CLASS_NUMBER);

    // bool
    Class::CLASS_BOOL = new Class("Bool",CLASS_OBJECT);
    Bool::kValueField = CLASS_BOOL->CreateField("value", CLASS_BOOL);

    // array
    Class::CLASS_ARRAY = new Class("Array", CLASS_OBJECT);
    Array::kLengthField = CLASS_ARRAY->CreateField("length", CLASS_NUMBER);
    Array::kDataField = CLASS_ARRAY->CreateField("data", CLASS_ARRAY);
  }

  Field* Class::CreateField(const std::string& name, Class* type){
    auto field = new Field(name, type, this);
    AddField(field);
    return field;
  }

  uint64_t Class::GetAllocationSize() const{
    uint64_t offset = sizeof(Instance);

    Class* cls = parent_;
    while(cls != nullptr && cls->HasParent()){
      offset += cls->GetAllocationSize();
      cls = cls->GetParent();
    }

    for(auto& field : fields_){
      field->SetOffset(offset);
      offset += kWordSize;
    }
    return offset;
  }

  Field* Class::GetField(const std::string& name) const{
    for(auto& field : fields_){
      if(field->GetName() == name)
        return field;
    }
    return nullptr;
  }

  void Class::VisitPointers(RawObjectPointerVisitor* vis){
    for(auto& field : fields_){
      if(!vis->Visit(field->raw_object()))
        return;
    }
  }

  void Class::VisitPointers(RawObjectPointerPointerVisitor* vis){
    for(auto& field : fields_){
      if(!vis->Visit(field->raw_object_ptr()))
        return;
    }
  }
}