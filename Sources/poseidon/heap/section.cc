#include "poseidon/raw_object.h"
#include "poseidon/heap/section.h"

namespace poseidon{
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