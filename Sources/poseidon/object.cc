#include "poseidon/object.h"

namespace poseidon {
#define REQUIRE_UNINITIALIZED_CLASS(Level) \
 LOG_IF(Level, kClass != nullptr) << "class `" << GetClassName() << "` is already initialized.";

#define REQUIRE_INITIALIZED_CLASS(Level) \
 LOG_IF(Level, kClass == nullptr) << "class `" << GetClassName() << "` is not initialized.";

#define DEFINE_INITIALIZE_CLASS(Name) \
 Class* Name::kClass = nullptr;       \
 void Name::InitializeClass() {       \
  REQUIRE_UNINITIALIZED_CLASS(FATAL); \
  kClass = Name::CreateClass();       \
 }

 DEFINE_INITIALIZE_CLASS(Object);
 Class* Object::CreateClass() {
   return new Class(kClassName, kTypeId, nullptr);
 }

 DEFINE_INITIALIZE_CLASS(Class);
 Class* Class::CreateClass() {
   REQUIRE_UNINITIALIZED_CLASS(FATAL);
   return new Class(kClassName, kTypeId, Object::GetClass());
 }

 DEFINE_INITIALIZE_CLASS(Field);
 Class* Field::CreateClass() {
   REQUIRE_UNINITIALIZED_CLASS(FATAL);
   return new Class(kClassName, kTypeId, Object::GetClass());
 }

 void Class::Initialize(){
   REQUIRE_UNINITIALIZED_CLASS(FATAL);

#define INITIALIZE_CLASS(Name) Name::InitializeClass();
   FOR_EACH_TYPE(INITIALIZE_CLASS);
#undef INITIALIZE_CLASS
 }

 Field* Class::CreateField(std::string name, Class* type) {
   auto field = new Field(std::move(name), type, this);
   fields_.push_back(field);
   return field;
 }

 ObjectSize Class::GetAllocationSize() const { //TODO: cache value
   auto offset = HasParent() ?
       GetParent()->GetAllocationSize() :
       static_cast<ObjectSize>(sizeof(Instance));
   for(auto field : fields_) {
     field->SetOffset(offset);
     offset += kWordSize;
   }
   return offset;
 }

 template<class T>
 T* Instance::New(Class* cls){
   DLOG(INFO) << "allocating " << (*cls);
   auto address = (uword)malloc(cls->GetAllocationSize());
   T fake;
   memcpy((void*) address, (void*) &fake, sizeof(T));
   memset((void*) (address + sizeof(T)), 0, cls->GetAllocationSize() - sizeof(T));
   return (T*)address;
 }

 DEFINE_INITIALIZE_CLASS(Null);
 Class* Null::CreateClass() {
   LOG_IF(FATAL, kClass != nullptr) << kClassName << " class is already initialized";
   auto cls = kClass = new Class(kClassName, kTypeId);
   return cls;
 }

 static Null* kNull = nullptr;
 Null* Null::Get() {
   CHECK_CLASS_IS_INITIALIZED()
   if(kNull == nullptr)
     kNull = New();
   return kNull;
 }

 DEFINE_INITIALIZE_CLASS(Bool);
 Field* Bool::kValueField = nullptr;

 Class* Bool::CreateClass() {
   REQUIRE_UNINITIALIZED_CLASS(FATAL);
   auto cls = new Class(kClassName, kTypeId);
   kValueField = cls->CreateField("value", cls);
   return cls;
 }

 static Bool* kTrue = nullptr;
 Bool* Bool::True() {
   CHECK_CLASS_IS_INITIALIZED(FATAL);
   if(kTrue == nullptr)
     kTrue = New(true);
   return kTrue;
 }

 static Bool* kFalse = nullptr;
 Bool* Bool::False() {
   CHECK_CLASS_IS_INITIALIZED(FATAL);
   if(kFalse == nullptr)
     kFalse = New(false);
   return kFalse;
 }

 DEFINE_INITIALIZE_CLASS(Number);
 Field* Number::kValueField = nullptr;

 Class* Number::CreateClass() {
   auto cls = new Class(kClassName, kTypeId);
   kValueField = cls->CreateField("value", cls);
   return cls;
 }

#define DEFINE_NUMBER_TYPE_CLASS(Name) \
 DEFINE_INITIALIZE_CLASS(Name);        \
 Class* Name::CreateClass() { REQUIRE_UNINITIALIZED_CLASS(FATAL); return new Class(kClassName, kTypeId, Number::GetClass()); } \

 FOR_EACH_INT_TYPE(DEFINE_NUMBER_TYPE_CLASS)
#undef DEFINE_NUMBER_TYPE_CLASS

 Class* Tuple::kClass = nullptr;
 Field* Tuple::kCarField = nullptr;
 Field* Tuple::kCdrField = nullptr;

 Class* Tuple::CreateClass() {
   REQUIRE_UNINITIALIZED_CLASS(FATAL);
   auto cls = new Class("Tuple", kTypeId);
   kCarField = cls->CreateField("car", Object::GetClass());
   kCdrField = cls->CreateField("cdr", Object::GetClass());
   return cls;
 }

 void Tuple::InitializeClass() {
   REQUIRE_UNINITIALIZED_CLASS(FATAL);
   kClass = Tuple::CreateClass();
 }
}