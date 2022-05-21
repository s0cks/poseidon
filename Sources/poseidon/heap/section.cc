#include "poseidon/raw_object.h"
#include "poseidon/heap/section.h"

namespace poseidon{
 uword AllocationSection::TryAllocate(int64_t size){
   return RawObject::TryAllocateIn(this, size);
 }

 void AllocationSection::VisitPointers(RawObjectVisitor* vis) const{
   return IteratePointers<AllocationSection, Iterator>(this, vis);
 }

 void AllocationSection::VisitPointers(RawObjectVisitorFunction vis) const{
   return IteratePointers<AllocationSection, Iterator>(this, vis);
 }

 void AllocationSection::VisitMarkedPointers(RawObjectVisitor* vis) const{
   return IterateMarkedPointers<AllocationSection, Iterator>(this, vis);
 }

 void AllocationSection::VisitMarkedPointers(RawObjectVisitorFunction vis) const{
   return IterateMarkedPointers<AllocationSection, Iterator>(this, vis);
 }
}