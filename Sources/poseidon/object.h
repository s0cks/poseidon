#ifndef PSDN_OBJECT_H
#define PSDN_OBJECT_H

#include "poseidon/type.h"
#include "poseidon/pointer.h"
#include "poseidon/heap/heap.h"

namespace poseidon {
 class Object{
   friend class Class;
   friend class Pointer;
  private:
   static constexpr const TypeId kTypeId = TypeId::kObjectTypeId;
   static constexpr const char* kClassName = "Object";
   static Class* kClass;

   static void InitializeClass();
  protected:
   Object() = default;

   virtual bool VisitPointers(RawObjectVisitor* vis) {
     return true;
   }

   static Class* CreateClass();
  public:
   virtual ~Object() = default;
   virtual TypeId GetTypeId() const = 0;

   uword GetStartingAddress() const {
     return (uword)this;
   }

   Pointer* raw_ptr() const {
     return (Pointer*)(GetStartingAddress() - sizeof(Pointer));
   }

   static inline const char* GetClassName() {
     return kClassName;
   }

   static inline Class* GetClass() {
     return kClass;
   }
 };

#define DEFINE_TYPE(Name) \
 public:                    \
  static constexpr const char* kClassName = #Name; \
  static const constexpr TypeId kTypeId = TypeId::k##Name##TypeId; \
  TypeId GetTypeId() const override { return kTypeId; }            \
  static Class* GetClass() { return kClass; }      \
  static ObjectSize GetClassAllocationSize() { CHECK_CLASS_IS_INITIALIZED(FATAL); return GetClass()->GetAllocationSize(); } \
 private:                 \
  static Class* kClass;   \
  static Class* CreateClass();                     \
  static void InitializeClass();

#define DEFINE_OBJECT(Name) \
  DEFINE_TYPE(Name);        \
 public:                    \
  void* operator new(size_t) noexcept { \
    CHECK_CLASS_IS_INITIALIZED(FATAL);  \
    LOG_IF(FATAL, !Heap::CurrentThreadHasHeap()) << "current thread `" << GetCurrentThreadName() << "` does not have a Heap."; \
    auto address = Heap::GetCurrentThreadHeap()->TryAllocateClassBytes(GetClass());                                            \
    LOG_IF(FATAL, address == UNALLOCATED) << "failed to allocate new " << kClassName;                                          \
    return (void*)address;  \
  }                         \
  template<class Z>         \
  void* operator new(size_t, Z* zone) noexcept {                                                                               \
    CHECK_CLASS_IS_INITIALIZED(FATAL);  \
    auto address = zone->TryAllocateClassBytes(GetClass());                                                                    \
    LOG_IF(FATAL, address == UNALLOCATED) << "failed to allocate new " << kClassName;                                          \
    return (void*)address;  \
  }                         \
  void operator delete(void*) noexcept { /* do nothing */ }                                                                    \

#define CHECK_CLASS_IS_INITIALIZED(Severity) \
  LOG_IF(FATAL, kClass == nullptr) << "Class " << kClassName << " is not initialized";

 class Class : public Object {
   friend class Pointer;
   friend class Instance;
  private:
   std::string name_;
   TypeId type_id_;
   Class* parent_;
   std::vector<Field*> fields_;
  protected:
   bool VisitPointers(RawObjectVisitor* vis) override {
     return true;
   }
  public:
   explicit Class(std::string name, const TypeId type_id, Class* parent = Object::GetClass()):
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

   bool HasParent() const {
     return parent_ != nullptr;
   }

   size_t GetFieldCount() const {
     return fields_.size();
   }

   Field* GetFieldAt(const size_t index) const {
     return fields_[index];
   }

   ObjectSize GetAllocationSize() const;
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

   DEFINE_TYPE(Class);
  public:
   static void Initialize();
 };

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
     offset_(0) { }

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

  DEFINE_TYPE(Field);
 };

 class Instance : public Object {
   friend class Pointer;
  protected:
   Class* type_;
   TypeId type_id_;

   explicit Instance(Class* type, const TypeId type_id = TypeId::kUnknownTypeId):
     Object(),
     type_(type),
     type_id_(type_id) {
     raw_ptr()->SetTypeId(type_id);
   }

   uword FieldAddrAtOffset(int64_t offset) const {
     return GetStartingAddress()  + offset;
   }

   Pointer** FieldAddr(const Field* field) const {
     return (Pointer**) FieldAddrAtOffset(field->GetOffset());
   }

   bool VisitPointers(RawObjectVisitor* vis) override {
     return true;
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
     auto ptr = (*FieldAddr(field));
     if(ptr == UNALLOCATED)
       return UNALLOCATED;
     return (Instance*) ptr->GetObjectPointerAddress();
   }

   void SetField(const Field* field, Pointer* value) {
     *FieldAddr(field) = value;
   }
  public:
   template<class T>
   static T* New(Class* cls);
 };

 class Null : public Instance {
   friend class Class;
  protected:
   Null():
     Instance(kClass, kTypeId) {
   }
  public:
   ~Null() override = default;

   friend std::ostream& operator<<(std::ostream& stream, const Null& value) {
     return stream << "Null()";
   }

   DEFINE_OBJECT(Null);
  public:
   template<class Z>
   static inline Null* TryAllocateIn(Z* zone) {
     return new (zone)Null();
   }

   static inline Null* New() {
     return new Null();
   }

   static Null* Get();
 };

 typedef bool RawBool;

 class Bool : public Instance {
   friend class Class;
   friend class Instance;
  private:
   static Field* kValueField;

   explicit Bool(const RawBool value):
     Instance(kClass, kTypeId) {
     Set(value);
   }
  public:
   ~Bool() override = default;

   void Set(const RawBool value) { //TODO: visible for testing
     CHECK_CLASS_IS_INITIALIZED(FATAL);
     *((RawBool*) FieldAddrAtOffset(kValueField->GetOffset())) = value;
   }

   RawBool Get() const {
     CHECK_CLASS_IS_INITIALIZED(FATAL);
     return *((RawBool*) FieldAddrAtOffset(kValueField->GetOffset()));
   }

   friend std::ostream& operator<<(std::ostream& stream, const Bool& value) {
     stream << "Bool(";
     stream << "value=" << (value.Get() ? "true" : "false");
     stream << ")";
     return stream;
   }

   DEFINE_OBJECT(Bool);
  public:
   template<class Z>
   static inline Bool*
   TryAllocateIn(Z* zone, const bool value) {
     return new (zone)Bool(value);
   }

   static inline Bool*
   New(const bool value) {
     return new Bool(value);
   }

   static Bool* True();
   static Bool* False();
 };

 typedef uword RawNumber;

 class Number : public Instance {
   friend class Class;
   friend class Instance;
  protected:
   static Field* kValueField;

   Number(Class* cls, const TypeId type_id, const RawNumber value = 0):
    Instance(cls, type_id) {
     Set(value);
   }

   explicit Number(const RawNumber value = 0):
    Instance(kClass, kTypeId) {
     Set(value);
   }
  public:
   ~Number() override = default;

   RawNumber Get() const {
     uword addr = ((uword)this) + Number::GetValueField()->GetOffset();
     return *((RawNumber*)addr);
   }

   void Set(const RawNumber value) {
     uword addr = ((uword)this) + Number::GetValueField()->GetOffset();
     *((RawNumber*)addr) = value;
   }

   friend std::ostream& operator<<(std::ostream& stream, const Number& value) {
     stream << "Number(";
     stream << "value=" << +value.Get() << ")";
     stream << ")";
     return stream;
   }

   DEFINE_OBJECT(Number);
  public:
   static inline Field*
   GetValueField() {
     CHECK_CLASS_IS_INITIALIZED(FATAL);
     return kValueField;
   }

   static inline Number* New(const RawNumber value = 0) {
     return new Number(value);
   }
 };

 typedef uint8_t RawByte;
 typedef uint8_t RawUInt8;

 class UInt8 : public Number {
   friend class Class;
  protected:
   explicit UInt8(const RawUInt8 value):
    Number(kClass, kTypeId, value){
   }

   static inline int
   CompareRaw(const RawUInt8 lhs, const RawUInt8 rhs) {
     if(lhs < rhs)
       return -1;
     else if(lhs > rhs)
       return +1;
     return 0;
   }
  public:
   ~UInt8() override = default;

   RawUInt8 Get() const {
     return static_cast<RawUInt8>(Number::Get());
   }

   void Set(const RawUInt8 value) {
     return Number::Set(static_cast<RawNumber>(value));
   }

   int Compare(const RawUInt8 rhs) const {
     return CompareRaw(Get(), rhs);
   }

   bool Equals(const RawUInt8 rhs) const {
     return CompareRaw(Get(), rhs) == 0;
   }

   DEFINE_OBJECT(UInt8);
  public:
   static inline UInt8* New(const RawUInt8 value = 0) {
     return new UInt8(value);
   }

   template<class Z>
   static inline UInt8* TryAllocateIn(Z* zone, const RawUInt8 value = 0) {
     return new (zone)UInt8(value);
   }
 };

 typedef int8_t RawInt8;

 class Int8 : public Number {
   friend class Class;
  protected:
   explicit Int8(const RawInt8 value):
     Number(kClass, kTypeId, value){
   }
  public:
   ~Int8() override = default;

   RawInt8 Get() const {
     return static_cast<RawInt8>(Number::Get());
   }

   void Set(const RawInt8 value) {
     return Number::Set(static_cast<RawNumber>(value));
   }

   DEFINE_OBJECT(Int8);
  public:
   static inline Int8* New(const RawInt8 value = 0) {
     return new Int8(value);
   }

   template<class Z>
   static inline Int8* TryAllocateIn(Z* zone, const RawInt8 value = 0) {
     return new (zone)Int8(value);
   }
 };

 typedef uint16_t RawUInt16;

 class UInt16 : public Number{
   friend class Class;
  private:
   explicit UInt16(const RawUInt16 value = 0):
    Number(kClass, kTypeId, value) {
   }
  public:
   ~UInt16() override = default;

   RawUInt16 Get() const {
     return static_cast<RawUInt16>(Number::Get());
   }

   void Set(const RawUInt16 value) {
     Number::Set(static_cast<RawNumber>(value));
   }

   DEFINE_OBJECT(UInt16);
  public:
   static inline UInt16* New(const RawUInt16 value = 0) {
     return new UInt16(value);
   }

   template<class Z>
   static inline UInt16* TryAllocateIn(Z* zone, const RawUInt16 value = 0) {
     return new (zone)UInt16(value);
   }
 };

 typedef int16_t RawInt16;

 class Int16 : public Number {
   friend class Class;
  private:
   explicit Int16(const RawInt16 value = 0):
     Number(kClass, kTypeId, value) {
   }
  public:
   ~Int16() override = default;

   RawInt16 Get() const {
     return static_cast<RawInt16>(Number::Get());
   }

   void Set(const RawInt16 value) {
     Number::Set(static_cast<RawNumber>(value));
   }

   DEFINE_OBJECT(Int16);
  public:
   static inline Int16* New(const RawInt16 value = 0) {
     return new Int16(value);
   }

   template<class Z>
   static inline Int16* TryAllocateIn(Z* zone, const RawInt16 value = 0) {
     return new (zone)Int16(value);
   }
 };

 typedef uint32_t RawUInt32;

 class UInt32 : public Number{
   friend class Class;
  private:
   explicit UInt32(const RawUInt32 value = 0):
     Number(kClass, kTypeId, value) {
   }
  public:
   ~UInt32() override = default;

   RawUInt32 Get() const {
     return static_cast<RawUInt32>(Number::Get());
   }

   void Set(const RawUInt32 value) {
     Number::Set(static_cast<RawNumber>(value));
   }

  DEFINE_OBJECT(UInt32);
  public:
   static inline UInt32* New(const RawUInt32 value = 0) {
     return new UInt32(value);
   }

   template<class Z>
   static inline UInt32* TryAllocateIn(Z* zone, const RawUInt32 value = 0) {
     return new (zone)UInt32(value);
   }
 };

 typedef int32_t RawInt32;

 class Int32 : public Number{
   friend class Class;
  private:
   explicit Int32(const RawInt32 value = 0):
       Number(kClass, kTypeId, value) {
   }
  public:
   ~Int32() override = default;

   RawInt32 Get() const {
     return static_cast<RawInt32>(Number::Get());
   }

   void Set(const RawInt32 value) {
     Number::Set(static_cast<RawNumber>(value));
   }

  DEFINE_OBJECT(Int32);

   friend std::ostream& operator<<(std::ostream& stream, const Int32& value) {
     stream << "Int32(";
     stream << "value=" << value.Get();
#if PSDN_DEBUG
     stream << ", ";
     stream << "ptr=" << (*value.raw_ptr());
#endif //PSDN_DEBUG
     stream << ")";
     return stream;
   }
  public:
   static inline Int32* New(const RawUInt32 value = 0) {
     return new Int32(value);
   }

   template<class Z>
   static inline Int32* TryAllocateIn(Z* zone, const RawInt32 value = 0) {
     return new (zone)Int32(value);
   }
 };

 typedef uint64_t RawUInt64;

 class UInt64 : public Number{
   friend class Class;
  private:
   explicit UInt64(const RawUInt64 value = 0):
    Number(kClass, kTypeId, value) {
     Set(value);
   }
  public:
   ~UInt64() override = default;

   RawUInt64 Get() const {
     return static_cast<RawUInt64>(Number::Get());
   }

   void Set(const RawUInt64 value) {
     return Number::Set(static_cast<RawNumber>(value));
   }

   DEFINE_OBJECT(UInt64);
  public:
   static inline UInt64* New(const RawUInt64 value = 0) {
     return new UInt64(value);
   }

   template<class Z>
   static inline UInt64* TryAllocateIn(Z* zone, const RawUInt64 value = 0) {
     return new (zone)UInt64(value);
   }
 };

 typedef int64_t RawInt64;

 class Int64 : public Number {
   friend class Class;
  private:
   explicit Int64(const RawInt64 value = 0):
    Number(kClass, kTypeId, value) {
     Set(value);
   }
  public:
   ~Int64() override = default;

   RawInt64 Get() const {
     return static_cast<RawInt64>(Number::Get());
   }

   void Set(const RawInt64 value = 0) {
     return Number::Set(static_cast<RawNumber>(value));
   }

   DEFINE_OBJECT(Int64);
  public:
   static inline Int64* New(const RawInt64 value = 0) {
     return new Int64(value);
   }

   template<class Z>
   static inline Int64* TryAllocateIn(Z* zone, const RawInt64 value = 0) {
     return new (zone)Int64(value);
   }
 };

 class Tuple : public Instance {
   friend class Class;
  protected:
   static Field* kCarField;
   static Field* kCdrField;

   explicit Tuple():
     Instance(kClass, kTypeId) {
     SetCar(UNALLOCATED);
     SetCdr(UNALLOCATED);
   }
  public:
   ~Tuple() override = default;

   void SetCar(Pointer* ptr = UNALLOCATED) {
     return SetField(kCarField, ptr);
   }

   template<class T>
   void SetCar(T* value) {
     return SetCar(value->raw_ptr());
   }

   bool HasCar() const {
     return GetField(kCarField) != UNALLOCATED;
   }

   Pointer* GetCarPointer() const {
     return HasCar() ? GetField(kCarField)->raw_ptr() : UNALLOCATED;
   }

   template<class T>
   T* GetCar() const {
     return (T*) GetField(kCarField);
   }

   Pointer* GetCdrPointer() const {
     return HasCdr() ? GetField(kCdrField)->raw_ptr() : UNALLOCATED;
   }

   void SetCdr(Pointer* ptr = UNALLOCATED) {
     return SetField(kCdrField, ptr);
   }

   template<class T>
   void SetCdr(T* value) {
     return SetCdr(value->raw_ptr());
   }

   bool HasCdr() const {
     return GetField(kCdrField) != UNALLOCATED;
   }

   template<class T>
   T* GetCdr() const {
     return (T*)GetField(kCdrField);
   }

   friend std::ostream& operator<<(std::ostream& stream, const Tuple& value) {
     stream << "Tuple(";
     if(value.HasCar())
       stream << "car=" << (*value.GetField(kCarField)->raw_ptr());
     if(value.HasCdr())
       stream << "cdr=" << (*value.GetField(kCdrField)->raw_ptr());
     stream << ")";
     return stream;
   }

   bool VisitPointers(RawObjectVisitor* vis) override {
     if(!vis->Visit(GetCarPointer())) {
       DLOG(WARNING) << "couldn't visit car: " << (*GetCarPointer());
       return false;
     }

     if(!vis->Visit(GetCdrPointer())) {
       DLOG(WARNING) << "couldn't visit cdr: " << (*GetCdrPointer());
       return false;
     }
     return true;
   }

  DEFINE_OBJECT(Tuple);
  public:
   static inline Tuple* New() {
     return new Tuple();
   }

   template<class Z>
   static inline Tuple* TryAllocateIn(Z* zone) {
     return new (zone)Tuple();
   }
 };
}

#endif //PSDN_OBJECT_H