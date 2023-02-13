#include "pointer.h"
#include "poseidon/object.h"

namespace poseidon{
 ObjectSize Pointer::VisitPointers(RawObjectVisitor* vis) {
   auto instance_size = GetPointerSize();
   switch(GetTypeId()) {
#define VISIT_TYPE_POINTERS(Name) \
     case TypeId::k##Name##TypeId: return ((Name*)GetObjectPointerAddress())->VisitPointers(vis) ? instance_size : 0;
     FOR_EACH_TYPE(VISIT_TYPE_POINTERS)
     default:
       break;
   }
   DLOG(INFO) << "visiting pointers in " << (*this);
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