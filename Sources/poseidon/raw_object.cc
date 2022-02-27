#include "raw_object.h"
#include "object.h"

namespace poseidon{
 uint64_t RawObject::VisitPointers(RawObjectPointerPointerVisitor* vis) const{
   uint64_t instance_size = GetPointerSize();
   auto from = (uword)(GetObjectPointerAddress() + sizeof(Instance) + sizeof(uword));
   auto to = from + instance_size;
   auto first = (RawObjectPtr*)from;
   auto last = (RawObjectPtr*)to;
   for(auto current = first; current <= last; current++){
     if(Allocator::GetHeap()->Contains((uword)*current) && !vis->Visit(current))
       break;
   }
   return instance_size;
 }
}