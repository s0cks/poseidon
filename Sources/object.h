#ifndef POSEIDON_OBJECT_H
#define POSEIDON_OBJECT_H

#include <string>
#include <utility>
#include <vector>
#include <functional>
#include <algorithm>
#include <glog/logging.h>

#include "utils.h"
#include "common.h"
#include "raw_object.h"

namespace poseidon{
  class Object;
  class ObjectPointerVisitor{
   protected:
    ObjectPointerVisitor() = default;
   public:
    virtual ~ObjectPointerVisitor() = default;
    virtual bool Visit(Object* obj) = 0;
  };

  class Object{
    friend class RawObject;
   protected:
    uword raw_ptr_;

    RawObject* raw_object() const{
      return (RawObject*)raw_ptr_;
    }

    RawObject** raw_object_ptr() const{
      return (RawObject**)&raw_ptr_;
    }

    virtual void VisitPointers(RawObjectPointerVisitor* vis){}
    virtual void VisitPointers(RawObjectPointerPointerVisitor* vis){}
   public:
    Object() = default;
    virtual ~Object() = default;
    virtual std::string ToString() const = 0;
  };

  class Field;
  class Class : public Object{
    friend class Allocator;
   private:
    std::string name_;
    Class* parent_;
    std::vector<Field*> fields_;
   protected:
    void VisitPointers(RawObjectPointerVisitor* vis) override;
    void VisitPointers(RawObjectPointerPointerVisitor* vis) override;
   public:
    Class(std::string name, Class* parent):
      Object(),
      name_(std::move(name)),
      parent_(parent),
      fields_(){
    }
    ~Class() override = default;

    uint64_t GetAllocationSize() const;

    Class* GetParent() const{
      return parent_;
    }

    bool HasParent() const{
      return parent_ != nullptr;
    }

    void AddField(Field* field){
      fields_.push_back(field);
    }

    std::string ToString() const override{
      NOT_IMPLEMENTED(ERROR);
      std::stringstream ss;
      ss << "Class(";
      ss << ")";
      return ss.str();
    }

    Field* GetField(const std::string& name) const;

    static void Initialize();

    static Class* CLASS_OBJECT;
    static Class* CLASS_CLASS;
    static Class* CLASS_FIELD;

    static Class* CLASS_NUMBER;
    static Class* CLASS_BYTE;
    static Class* CLASS_SHORT;
    static Class* CLASS_INT;
    static Class* CLASS_LONG;
  };

  class Field : public Object{
    friend class Class;
   private:
    std::string name_;
    Class* type_;
    Class* owner_;
    uword offset_;

    void SetOffset(uword offset){
      offset_ = offset;
    }
   public:
    Field(std::string name, Class* type, Class* owner):
      Object(),
      name_(std::move(name)),
      type_(type),
      owner_(owner),
      offset_(0){
    }
    ~Field() override = default;

    std::string GetName() const{
      return name_;
    }

    Class* GetType() const{
      return type_;
    }

    Class* GetOwner() const{
      return owner_;
    }

    uword GetOffset() const{
      return offset_;
    }

    std::string ToString() const override{
      NOT_IMPLEMENTED(ERROR);
      std::stringstream ss;
      ss << "Field(";
      ss << ")";
      return ss.str();
    }
  };

  class Instance : public Object{
   private:
    Class* type_;
   protected:
    explicit Instance(Class* type):
      Object(),
      type_(type){
    }

    Instance** FieldAddressAtOffset(uword offset) const{
      return reinterpret_cast<Instance**>(reinterpret_cast<uword>(this) + offset);
    }

    inline Instance** FieldAddress(Field* field) const{
      return FieldAddressAtOffset(field->GetOffset());
    }
   public:
    ~Instance() override = default;

    Class* GetType() const{
      return type_;
    }
  };

  template<typename T>
  class Number : public Instance{
   public:
    static Field* kValueField;
   protected:
    explicit Number(Class* cls):
      Instance(cls){
    }
   public:
    ~Number() override = default;

    T Get() const{
      return *((T*)FieldAddress(kValueField));
    }

    void Set(const T& val){
      *((T*) FieldAddress(kValueField)) = val;
    }
  };

  class Byte : public Number<uint8_t>{
   public:
    Byte(): Number<uint8_t>(Class::CLASS_BYTE){}
    explicit Byte(const uint8_t& val):
      Byte(){
      Set(val);
    }
    ~Byte() override = default;

    std::string ToString() const override{
      std::stringstream ss;
      ss << "Byte(";
      ss << "value=" << Get();
      ss << ")";
      return ss.str();
    }

    Byte& operator=(const uint8_t& val){
      Set(val);
      return *this;
    }

    friend std::ostream& operator<<(std::ostream& stream, const Byte& val){
      return stream << val.ToString();
    }
  };

  class Short : public Number<uint16_t>{
   public:
    Short(): Number<uint16_t>(Class::CLASS_SHORT){}
    explicit Short(const uint16_t& val):
      Short(){
      Set(val);
    }
    ~Short() override = default;

    std::string ToString() const override{
      std::stringstream ss;
      ss << "Short(";
      ss << "value=" << Get();
      ss << ")";
      return ss.str();
    }

    Short& operator=(const uint16_t& val){
      Set(val);
      return *this;
    }

    friend std::ostream& operator<<(std::ostream& stream, const Short& val){
      return stream << val.ToString();
    }
  };

  class Int : public Number<uint32_t>{
   public:
    Int(): Number<uint32_t>(Class::CLASS_NUMBER){}
    Int(const uint32_t& val):
      Int(){
      Set(val);
    }
    ~Int() override = default;

    std::string ToString() const override{
      std::stringstream ss;
      ss << "Int(";
      ss << "value=" << Get();
      ss << ")";
      return ss.str();
    }

    Int& operator=(const uint32_t& val){
      Set(val);
      return *this;
    }

    friend std::ostream& operator<<(std::ostream& stream, const Int& val){
      return stream << val.ToString();
    }
  };

  class Long : public Number<uint64_t>{
   public:
    Long(): Number<uint64_t>(Class::CLASS_LONG){}
    Long(const uint64_t& val):
      Long(){
      Set(val);
    }
    ~Long() override = default;

    std::string ToString() const override{
      std::stringstream ss;
      ss << "Long(";
      ss << "value=" << Get();
      ss << ")";
      return ss.str();
    }

    Long& operator=(const uint64_t& val){
      Set(val);
      return *this;
    }

    friend std::ostream& operator<<(std::ostream& stream,const Long& val){
      return stream << val.ToString();
    }
  };
}

#endif//POSEIDON_OBJECT_H