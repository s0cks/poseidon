#include "poseidon/type.h"

namespace poseidon {
 Class* Class::kObjectClass = nullptr;
 Class* Class::kNullClass = nullptr;
 Class* Class::kBoolClass = nullptr;
 Class* Class::kByteClass = nullptr;
 Class* Class::kShortClass = nullptr;
 Class* Class::kIntClass = nullptr;
 Class* Class::kLongClass = nullptr;
 Class* Class::kNumberClass = nullptr;
 Class* Class::kTupleClass = nullptr;

 void Class::Initialize(){
   if(kObjectClass != nullptr) { //TODO: handle better
     DLOG(WARNING) << "already initialized.";
     return;
   }

   kObjectClass = new Class("Object", TypeId::kObjectTypeId, nullptr);
   kBoolClass = Bool::CreateClass();
   kNumberClass = Number::CreateClass();
   kByteClass = Byte::CreateClass();
   kShortClass = Short::CreateClass();
   kIntClass = Int::CreateClass();
   kLongClass = Long::CreateClass();
   kTupleClass = Tuple::CreateClass();
 }

 Field* Class::CreateField(std::string name, Class* type) {
   auto field = new Field(std::move(name), type, this);
   fields_.push_back(field);
   return field;
 }

 int64_t Class::GetAllocationSize() const{ //TODO: cache value
   int64_t offset = sizeof(Instance);
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

 Class* Null::kClass = nullptr;

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

 Class* Bool::kClass = nullptr;
 Field* Bool::kValueField = nullptr;

 Class* Bool::CreateClass() {
   LOG_IF(FATAL, kClass != nullptr) << kClassName << " class is already initialized";
   auto cls = kClass = new Class(kClassName, kTypeId);
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

 Class* Number::kClass = nullptr;
 Field* Number::kValueField = nullptr;

 void* Number::operator new(size_t) noexcept {
   if(kClass == nullptr)
     LOG(FATAL) << "Number class not initialized";
   return malloc(kClass->GetAllocationSize()); //TODO: change to heap
 }

 void Number::operator delete(void* ptr) noexcept {
   // do nothing?
   free(ptr); //TODO: change to heap
 }

 Class* Number::CreateClass() {
   auto cls = kClass = new Class("Number", kTypeId);
   kValueField = cls->CreateField("value", cls);
   return cls;
 }

 Number::Number(const uword value):
     Instance(kClass, kTypeId) {
   Set(value);
 }

 uword Number::Get() const {
   return *((uword*) (((uword) this) + kValueField->GetOffset()));
 }

 void Number::Set(const uword value) {
   *((uword*) (((uword) this) + kValueField->GetOffset())) = value;
 }

 Class* Byte::kClass = nullptr;
 Field* Byte::kValueField = nullptr;

 Class* Byte::CreateClass() {
   LOG_IF(FATAL, kClass != nullptr) << kClassName << " class is already initialized";
   auto cls = kClass = new Class(kClassName, kTypeId, Class::kNumberClass);
   kValueField = cls->CreateField("value", cls);
   return cls;
 }

 Class* Short::kClass = nullptr;
 Field* Short::kValueField = nullptr;

 Class* Short::CreateClass() {
   LOG_IF(FATAL, kClass != nullptr) << kClassName << " is already initialized";
   auto cls = kClass = new Class(kClassName, kTypeId, Class::kNumberClass);
   kValueField = cls->CreateField("value", cls);
   return cls;
 }

 Class* Int::kClass = nullptr;
 Field* Int::kValueField = nullptr;

 Class* Int::CreateClass() {
   LOG_IF(FATAL, kClass != nullptr) << kClassName << " class is already initialized";
   auto cls = kClass = new Class(kClassName, kTypeId, Class::kNumberClass);
   kValueField = cls->CreateField("value", cls);
   return cls;
 }

 Class* Long::kClass = nullptr;
 Field* Long::kValueField = nullptr;

 Class* Long::CreateClass() {
   LOG_IF(FATAL, kClass != nullptr) << kClassName << " class is already initialized";
   auto cls = kClass = new Class(kClassName, kTypeId, Class::kNumberClass);
   kValueField = cls->CreateField("value", cls);
   return cls;
 }

 Class* Tuple::kClass = nullptr;
 Field* Tuple::kCarField = nullptr;
 Field* Tuple::kCdrField = nullptr;

 void* Tuple::operator new(const size_t) noexcept {
   LOG_IF(FATAL, kClass == nullptr) << "Tuple class not initialized";
   auto heap = Heap::GetCurrentThreadHeap();
   auto address = heap->TryAllocateClassBytes(kClass);
   LOG_IF(FATAL, address == UNALLOCATED) << "cannot allocate Tuple";
   return ((Pointer*)address)->GetPointer();
 }

 void Tuple::operator delete(void*) noexcept {
   // do nothing
 }

 Class* Tuple::CreateClass() {
   auto cls = kClass = new Class("Tuple", kTypeId);
   kCarField = cls->CreateField("car", Class::kObjectClass);
   kCdrField = cls->CreateField("cdr", Class::kObjectClass);
   return cls;
 }
}