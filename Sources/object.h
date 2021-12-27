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
    friend class Array;
    friend class RawObject;
    friend class LocalBase;
    friend class Allocator;
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
    virtual void Finalize(){}
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

    static Class* CLASS_BOOL;
    static Class* CLASS_ARRAY;
    static Class* CLASS_STRING;
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

  class Array;
  class Instance : public Object{
   private:
    Class* type_;
   protected:
    explicit Instance(Class* type):
      Object(),
      type_(type){
    }

    inline uword** FieldAddressAtOffset(uword offset) const{
      return (uword**)(raw_object()->GetPointerStartAddress() + offset);
    }

    inline uword** FieldAddress(Field* field) const{
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

    static inline Array* NewArrayInstance(const uint64_t& length);

    template<typename T>
    static inline Local<T> NewLocalInstance(Class* cls){
      auto local = Allocator::AllocateLocal<T>();
      local.Set(NewInstance<T>(cls));
      return local;
    }

    static inline Local<Array> NewLocalArrayInstance(const uint64_t& length){
      auto local = Allocator::AllocateLocal<Array>();
      local.Set(NewArrayInstance(length));
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

    static inline Byte*
    New(){
      return Instance::NewInstance<Byte>(Class::CLASS_BYTE);
    }

    static inline Byte*
    New(const uint8_t& val){
      auto instance = New();
      instance->Set(val);
      return instance;
    }

    static inline Local<Byte>
    NewLocal(){
      return Instance::NewLocalInstance<Byte>(Class::CLASS_BYTE);
    }

    static inline Local<Byte>
    NewLocal(const uint8_t& val){
      auto local = NewLocal();
      local->Set(val);
      return local;
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

    static inline Short*
    New(){
      return Instance::NewInstance<Short>(Class::CLASS_SHORT);
    }

    static inline Short*
    New(const uint16_t& val){
      auto instance = New();
      instance->Set(val);
      return instance;
    }

    static inline Local<Short>
    NewLocal(){
      return Instance::NewLocalInstance<Short>(Class::CLASS_SHORT);
    }

    static inline Local<Short>
    NewLocal(const uint32_t& val){
      auto local = NewLocal();
      local->Set(val);
      return local;
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

    static inline Int*
    New(const uint32_t& val){
      auto instance = New();
      instance->Set(val);
      return instance;
    }

    static inline Local<Int>
    NewLocal(){
      return Instance::NewLocalInstance<Int>(Class::CLASS_INT);
    }

    static inline Local<Int>
    NewLocal(const uint32_t& val){
      auto local = NewLocal();
      local->Set(val);
      return local;
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

    static inline Long*
    New(){
      return Instance::NewInstance<Long>(Class::CLASS_LONG);
    }

    static inline Long*
    New(const uint64_t& val){
      auto instance = New();
      instance->Set(val);
      return instance;
    }

    static inline Local<Long>
    NewLocal(){
      return Instance::NewLocalInstance<Long>(Class::CLASS_LONG);
    }

    static inline Local<Long>
    NewLocal(const uint64_t& val){
      auto local = NewLocal();
      local->Set(val);
      return local;
    }
  };

  class String : public Instance{
   public:
    static Field* kLengthField;
    static Field* kDataField;
   public:
    String():
      Instance(Class::CLASS_STRING){
    }
    ~String() override = default;

    uint64_t GetLength() const{
      return *((uint64_t*)FieldAddress(kLengthField));
    }

    uint8_t* GetData() const{
      return (uint8_t*)FieldAddress(kDataField);
    }

    std::string Get() const{
      NOT_IMPLEMENTED(ERROR);
      return "";
    }

    std::string ToString() const override{
      std::stringstream ss;
      ss << "String(";
      ss << "value=" << Get();
      return ss.str();
    }
  };

  class Array : public Instance{
    friend class Instance;
   public:
    static Field* kLengthField;
    static Field* kDataField;
   protected:
    void VisitPointers(RawObjectPointerVisitor* vis) override{
      for(auto idx = 0; idx < GetLength(); idx++){
        if(!vis->Visit(*GetObjectAt(idx)))
          return;
      }
    }

    void VisitPointers(RawObjectPointerPointerVisitor* vis) override{
      for(auto idx = 0; idx < GetLength(); idx++){
        if(!vis->Visit(GetObjectAt(idx)))
          return;
      }
    }

    static inline uint64_t
    GetOffsetForIndex(const uint64_t& index){
      return kDataField->GetOffset() + (sizeof(uword) * index);
    }

    void SetLength(const uint64_t& val){
      *((uint64_t*) FieldAddress(kLengthField)) = val;
    }

    void SetObjectAt(const uint64_t& idx, RawObject* val){
      *((RawObject**)FieldAddressAtOffset(GetOffsetForIndex(idx))) = val;
    }

    RawObject** GetObjectAt(const uint64_t& idx) const{
      return (RawObject**)FieldAddressAtOffset(GetOffsetForIndex(idx));
    }
   public:
    Array():
      Instance(Class::CLASS_ARRAY){
    }

    uint64_t GetLength() const{
      return *((uint64_t*) FieldAddress(kLengthField));
    }

    template<typename T>
    T* GetAt(const uint64_t& idx) const{
      return (T*)(*GetObjectAt(idx))->GetPointer();
    }

    template<typename T>
    void SetAt(const uint64_t& idx, const T* val){
      SetObjectAt(idx, val->raw_object());
    }
  };

  class Bool : public Instance{
   public:
    static Field* kValueField;
   protected:
    Bool():
      Instance(Class::CLASS_BOOL){
    }

    void Set(const bool& val){
      DLOG(INFO) << "setting bool value @" << (void*)(raw_object()->GetPointer() + kValueField->GetOffset());
      *((bool*) FieldAddress(kValueField)) = val;
    }
   public:
    ~Bool() override = default;

    bool Get() const{
      return *((bool*) FieldAddress(kValueField));
    }

    explicit operator bool() const{
      return Get();
    }

    std::string ToString() const override{
      std::stringstream ss;
      ss << "Bool(";
      ss << "value=" << (Get() ? "true" : "false");
      ss << ")";
      return ss.str();
    }

    static inline Bool* New(){
      return Instance::NewInstance<Bool>(Class::CLASS_BOOL);
    }

    static inline Bool* New(const bool val){
      auto instance = New();
      instance->Set(val);
      return instance;
    }

    static inline Local<Bool>
    NewLocal(){
      return Instance::NewLocalInstance<Bool>(Class::CLASS_BOOL);
    }

    static inline Local<Bool>
    NewLocal(const bool val){
      auto local = NewLocal();
      local->Set(val);
      return local;
    }
  };

  Array* Instance::NewArrayInstance(const uint64_t& length){
    auto size = Class::CLASS_ARRAY->GetAllocationSize() + (sizeof(uword) * length);
    auto data = Allocator::AllocateRawObject(size);

    auto fake = new Instance(Class::CLASS_ARRAY);
    fake->SetRawPointer((uword)data);
    memcpy(data->GetPointer(), fake, sizeof(Instance));
    delete fake;

    auto val = (Array*)data->GetPointer();
    val->SetLength(length);
    return val;
  }
}

#endif//POSEIDON_OBJECT_H