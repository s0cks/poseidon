#include "raw_object.h"
#include "type/instance.h"
#include "heap/heap.h"

namespace poseidon{
 ObjectSize RawObject::VisitPointers(RawObjectVisitor* vis) {
   auto instance_size = GetPointerSize();

   auto current = GetObjectPointerAddress() + sizeof(Instance);
   while(current < GetEndingAddress()) {
     auto ptr = (RawObject**)current;
     if((*ptr) != UNALLOCATED && !vis->Visit((*ptr)))
       return instance_size;
     current += kWordSize;
   }
   return instance_size;
 }
}