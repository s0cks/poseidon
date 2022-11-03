#include "poseidon/type/instance.h"

namespace poseidon {
 template<class T>
 T* Instance::New(Class* cls){
   DLOG(INFO) << "allocating " << (*cls);
   auto address = (uword)malloc(cls->GetAllocationSize());
   T fake;
   memcpy((void*) address, (void*) &fake, sizeof(T));
   memset((void*) (address + sizeof(T)), 0, cls->GetAllocationSize() - sizeof(T));
   return (T*)address;
 }
}