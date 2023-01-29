#include "pointer.h"
#include "poseidon/type.h"

namespace poseidon{
 ObjectSize Pointer::VisitPointers(RawObjectVisitor* vis) {
   auto instance_size = GetPointerSize();

   auto current = GetObjectPointerAddress() + sizeof(Instance);
   while(current < GetEndingAddress()) {
     auto ptr = (Pointer**)current;
     if((*ptr) != UNALLOCATED && !vis->Visit((*ptr)))
       return instance_size;
     current += kWordSize;
   }
   return instance_size;
 }
}