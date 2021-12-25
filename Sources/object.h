#ifndef POSEIDON_OBJECT_H
#define POSEIDON_OBJECT_H

#include <string>
#include <utility>
#include <vector>
#include <functional>
#include <algorithm>
#include <glog/logging.h>

#include "utils.h"
#include "local.h"
#include "common.h"
#include "allocator.h"
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
    friend class LocalBase;
    friend class LiveObjectForwarder;
   protected:
    uword raw_ptr_;

    Object():
      raw_ptr_(0){
    }

    RawObject* raw_object() const{
      return (RawObject*)raw_ptr_;
    }

    RawObject** raw_object_ptr() const{
      return (RawObject**)&raw_ptr_;
    }

    void SetRawPointer(const uword raw){
      raw_ptr_ = raw;
    }

    virtual void VisitPointers(RawObjectPointerVisitor* vis){}
    virtual void VisitPointers(RawObjectPointerPointerVisitor* vis){}
   public:
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
    inline void
    AddField(Field* field){
      fields_.push_back(field);
    }

    Field* CreateField(const std::string& name, Class* type);
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

    std::string GetName() const{
      return name_;
    }

    uint64_t GetAllocationSize() const;

    Class* GetParent() const{
      return parent_;
    }

    bool HasParent() const{
      return parent_ != nullptr;
    }

    std::string ToString() const override{
      NOT_IMPLEMENTED(ERROR);
      std::stringstream ss;
      ss << "Class(";
      ss << "name=" << GetName();
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
      return reinterpret_cast<Instance**>(reinterpret_cast<uword>(raw_object()->GetObjectPointer()) + offset);
    }

    inline Instance** FieldAddress(Field* field) const{
      return FieldAddressAtOffset(field->GetOffset());
    }
   public:
    ~Instance() override = default;

    Class* GetType() const{
      return type_;
    }

    std::string ToString() const override{
      std::stringstream ss;
      ss << "Instance(";
      ss << "type=" << GetType()->ToString();
      ss << ")";
      return ss.str();
    }

    template<typename T>
    static inline T* NewInstance(Class* cls){
      auto data = Allocator::AllocateRawObject(cls->GetAllocationSize());

      auto fake = new Instance(cls);
      fake->SetRawPointer((uword)data);
      memcpy(data->GetPointer(), fake, sizeof(Instance));
      delete fake;

      return (T*)data->GetPointer();
    }

    template<typename T>
    static inline Local<T> NewLocalInstance(Class* cls){
      auto local = Allocator::AllocateLocal<T>();
      local.Set(NewInstance<T>(cls));
      return local;
    }
  };

  class Number : public Instance{
   public:
    static Field* kValueField;
   protected:
    explicit Number(Class* cls):
      Instance(cls){
    }
   public:
    ~Number() override = default;
  };

  class Byte : public Number{
   public:
    Byte(): Number(Class::CLASS_BYTE){}
    explicit Byte(const uint8_t& val):
      Byte(){
      Set(val);
    }
    ~Byte() override = default;

    uint8_t Get() const{
      return *((uint8_t*)FieldAddress(kValueField));
    }

    void Set(const uint8_t& val){
      *((uint8_t*) FieldAddress(kValueField)) = val;
    }

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

  class Short : public Number{
   public:
    Short(): Number(Class::CLASS_SHORT){}
    explicit Short(const uint16_t& val):
      Short(){
      Set(val);
    }
    ~Short() override = default;

    uint16_t Get() const{
      return *((uint16_t*)FieldAddress(kValueField));
    }

    void Set(const uint16_t& val){
      *((uint16_t*) FieldAddress(kValueField)) = val;
    }

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

  class Int : public Number{
   public:
    Int(): Number(Class::CLASS_NUMBER){}
    explicit Int(const uint32_t& val):
      Int(){
      Set(val);
    }
    ~Int() override = default;

    uint32_t Get() const{
      DLOG(INFO) << "raw pointer: " << raw_object();
      return raw_object() ? *((uint32_t*)FieldAddress(kValueField)) : 0;//TODO: fix
    }

    void Set(const uint32_t& val){
      *((uint32_t*) FieldAddress(kValueField)) = val;
    }

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

    static inline Int*
    New(){
      return Instance::NewInstance<Int>(Class::CLASS_INT);
    }

    static inline Local<Int>
    NewLocal(){
      return Instance::NewLocalInstance<Int>(Class::CLASS_INT);
    }
  };

  class Long : public Number{
   public:
    Long(): Number(Class::CLASS_LONG){}
    explicit Long(const uint64_t& val):
      Long(){
      Set(val);
    }
    ~Long() override = default;

    uint64_t Get() const{
      return *((uint64_t*)FieldAddress(kValueField));
    }

    void Set(const uint64_t& val){
      *((uint64_t*) FieldAddress(kValueField)) = val;
    }

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