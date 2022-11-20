#include "poseidon/type/class.h"
#include "poseidon/type/instance.h"

#include "poseidon/type/bool.h"
#include "poseidon/type/int.h"
#include "poseidon/type/byte.h"
#include "poseidon/type/long.h"
#include "poseidon/type/short.h"
#include "poseidon/type/tuple.h"
#include "poseidon/type/number.h"

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
}