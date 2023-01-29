#ifndef PSDN_TYPE_H
#define PSDN_TYPE_H

#include "poseidon/pointer.h"
#include "poseidon/heap/heap.h"

namespace poseidon {
#define FOR_EACH_TYPE(V) \
 V(Object)               \
 V(Null)                 \
 V(Bool)                 \
 V(Byte)                 \
 V(Short)                \
 V(Int)                  \
 V(Long)                 \
 V(Class)                \
 V(Field)                \
 V(Number)               \
 V(Tuple)

#define DEFINE_TYPE_ID(Name) k##Name##TypeId,
 enum class TypeId : word {
   kUnknownTypeId = 0,
   FOR_EACH_TYPE(DEFINE_TYPE_ID)
   kMaxTypeId,
 };
#undef DEFINE_TYPE_ID

#define FORWARD_DECLARE(Name) class Name;
 FOR_EACH_TYPE(FORWARD_DECLARE)
#undef FORWARD_DECLARE

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
  void* operator new(size_t) noexcept { \
    CHECK_CLASS_IS_INITIALIZED(FATAL);  \
    LOG_IF(FATAL, !Heap::CurrentThreadHasHeap()) << "current thread `" << GetCurrentThreadName() << "` does not have a Heap."; \
    auto address = Heap::GetCurrentThreadHeap()->TryAllocateClassBytes(GetClass());                                            \
    LOG_IF(FATAL, address == UNALLOCATED) << "failed to allocate new " << kClassName;                                          \
    return (void*)address;  \
  }                         \
  template<class Z>         \
  void* operator new(size_t, Z* zone) noexcept {                                                                               \
    CHECK_CLASS_IS_INITIALIZED(FATAL); \
    auto address = zone->TryAllocateClassBytes(GetClass());                                                                    \
    LOG_IF(FATAL, address == UNALLOCATED) << "failed to allocate new " << kClassName;                                          \
    return (void*)address;  \
  }                         \
  void operator delete(void*) noexcept { /* do nothing */ }                                                                    \
  static constexpr const char* kClassName = #Name; \
  static const constexpr TypeId kTypeId = TypeId::k##Name##TypeId; \
  TypeId GetTypeId() const override { return kTypeId; }            \
  static Class* GetClass() { return kClass; }      \
  static ObjectSize GetClassAllocationSize() { CHECK_CLASS_IS_INITIALIZED(FATAL); return GetClass()->GetAllocationSize(); } \
 private:                   \
  static Class* kClass;     \
  static Class* CreateClass();

#define CHECK_CLASS_IS_INITIALIZED(Severity) \
  LOG_IF(FATAL, kClass == nullptr) << "Class " << kClassName << " is not initialized";

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
     return GetStartingAddress()  + offset;
   }

   Pointer** FieldAddr(const Field* field) const {
     return (Pointer**) FieldAddrAtOffset(field->GetOffset());
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
  protected:
   static Field* kValueField;
  protected:
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

 typedef uint8_t RawByte;

 class Number : public Instance {
   friend class Class;
   friend class Instance;
  protected:
   static Field* kValueField;

   explicit Number(uword value);
   void Set(uword value);
  public:
   ~Number() override = default;

   uword Get() const;

   friend std::ostream& operator<<(std::ostream& stream, const Number& value) {
     stream << "Number(";
     stream << "value=" << +value.Get() << ")";
     stream << ")";
     return stream;
   }

   DEFINE_OBJECT(Number);
  public:
   static inline Number* New(const uword value) {
     return new Number(value);
   }

   static inline Number* New() {
     return New(0);
   }
 };

 class Byte : public Instance {
   friend class Class;
   friend class Instance;
  protected:
   static Field* kValueField;
  protected:
   explicit Byte(RawByte value):
     Instance(kClass, kTypeId) {
     Set(value);
   }
  public:
   ~Byte() override = default;

   RawByte Get() const {
     CHECK_CLASS_IS_INITIALIZED(FATAL);
     return *((RawByte*) FieldAddrAtOffset(kValueField->GetOffset()));
   }

   void Set(const RawByte value) {
     CHECK_CLASS_IS_INITIALIZED(FATAL);
     *((RawByte*) FieldAddrAtOffset(kValueField->GetOffset())) = value;
   }

   friend std::ostream& operator<<(std::ostream& stream, const Byte& value) {
     stream << "Byte(";
     stream << "value=" << +value.Get();
     stream << ")";
     return stream;
   }

   DEFINE_OBJECT(Byte);
  public:
   static inline Byte* New(const RawByte value = 0) {
     return new Byte(value);
   }

   template<class Z>
   static inline Byte* TryAllocateIn(Z* zone, const RawByte value = 0) {
     return new (zone)Byte(value);
   }
 };

 typedef uint16_t RawShort;

 class Short : public Instance {
   friend class Class;
   friend class Instance;
  protected:
   static Field* kValueField;

   explicit Short(RawShort value):
       Instance(kClass, kTypeId) {
     Set(value);
   }
  public:
   ~Short() override = default;

   RawShort Get() const {
     LOG_IF(FATAL, kClass == nullptr) << "Short class is not initialized";
     return *((RawShort*) FieldAddrAtOffset(kValueField->GetOffset()));
   }

   void Set(const RawShort value) {
     LOG_IF(FATAL, kClass == nullptr) << "Short class is not initialized";
     *((RawShort*) FieldAddrAtOffset(kValueField->GetOffset())) = value;
   }

   friend std::ostream& operator<<(std::ostream& stream, const Short& value) {
     stream << "Short(";
     stream << "value=" << +value.Get();
     stream << ")";
     return stream;
   }

  DEFINE_OBJECT(Short);
  public:
   template<class Z>
   static inline Short*
   TryAllocateIn(Z* zone, const RawShort value = 0) {
     return new (zone)Short(value);
   }

   static inline Short*
   New(const RawShort value = 0) {
     return new Short(value);
   }
 };

 typedef uint32_t RawInt;

 class Int : public Instance {
   friend class Class;
   friend class Instance;
  protected:
   static Field* kValueField;

   explicit Int(RawInt value):
       Instance(kClass, kTypeId) {
     Set(value);
   }
  public:
   ~Int() override = default;

   void Set(const RawInt value) {
     CHECK_CLASS_IS_INITIALIZED(FATAL);
     *((RawInt*) FieldAddrAtOffset(kValueField->GetOffset())) = value;
   }

   RawInt Get() const {
     CHECK_CLASS_IS_INITIALIZED(FATAL);
     return *((RawInt*) FieldAddrAtOffset(kValueField->GetOffset()));
   }

   friend std::ostream& operator<<(std::ostream& stream, const Int& value) {
     stream << "Int(";
     stream << "value=" << +value.Get();
     stream << ")";
     return stream;
   }

   friend bool operator==(const Int& lhs, const Int& rhs) {
     return Compare(lhs, rhs) == 0;
   }

   friend bool operator!=(const Int& lhs, const Int& rhs) {
     return Compare(lhs, rhs) != 0;
   }

   friend bool operator<(const Int& lhs, const Int& rhs) {
     return Compare(lhs, rhs) < 0;
   }

   friend bool operator>(const Int& lhs, const Int& rhs) {
     return Compare(lhs, rhs) > 0;
   }

  DEFINE_OBJECT(Int);
  public:
   void* operator new(size_t, const Region& region) noexcept {
     CHECK_CLASS_IS_INITIALIZED(FATAL);
     LOG_IF(FATAL, region.GetStartingAddress() <= UNALLOCATED) << "cannot allocate new " << kClassName << " @ " << region;
     LOG_IF(FATAL, region.GetSize() < GetClass()->GetAllocationSize()) << "cannot allocate " << kClassName << " @ " << region;
     return region.GetStartingAddressPointer();
   }

   static inline int
   Compare(const Int& lhs, const Int& rhs) {
     if(lhs.Get() < rhs.Get())
       return -1;
     else if(lhs.Get() > rhs.Get())
       return +1;
     PSDN_ASSERT(lhs.Get() == rhs.Get());
     return 0;
   }

   static inline Int* TryAllocateAt(const Region& region, const RawInt value = 0) {
     return new (region)Int(value);
   }

   template<class Z>
   static inline Int* TryAllocateIn(Z* zone, const RawInt value = 0) {
     return new (zone)Int(value);
   }

   static inline Int* New(const RawInt value = 0) {
     return new Int(value);
   }
 };

 typedef uint64_t RawLong;

 class Long : public Instance {
   friend class Class;
   friend class Instance;
  protected:
   static Field* kValueField;

   explicit Long(RawLong value):
       Instance(kClass, kTypeId) {
     Set(value);
   }
  public:
   ~Long() override = default;

   RawLong Get() const {
     CHECK_CLASS_IS_INITIALIZED(FATAL);
     return *((RawLong*) FieldAddrAtOffset(kValueField->GetOffset()));
   }

   void Set(const RawLong value) {
     CHECK_CLASS_IS_INITIALIZED(FATAL);
     *((RawLong*) FieldAddrAtOffset(kValueField->GetOffset())) = value;
   }

   friend std::ostream& operator<<(std::ostream& stream, const Long& value) {
     stream << "Long(";
     stream << "value=" << +value.Get();
     stream << ")";
     return stream;
   }

  DEFINE_OBJECT(Long);
  public:
   template<class Z>
   static inline Long*
   TryAllocateIn(Z* zone, const RawLong value = 0) {
     return new (zone)Long(value);
   }

   static inline Long*
   New(const RawLong value = 0) {
     return new Long(value);
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

   bool HasCar() const {
     return GetField(kCarField) != UNALLOCATED;
   }

   template<class T>
   T* GetCar() const {
     return (T*) GetField(kCarField);
   }

   void SetCdr(Pointer* ptr = UNALLOCATED) {
     return SetField(kCdrField, ptr);
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

  DEFINE_OBJECT(Tuple);
  public:
   static inline Tuple* New() {
     return new Tuple();
   }
 };
}

#endif //PSDN_TYPE_H