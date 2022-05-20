#include "poseidon/heap/zone.h"

namespace poseidon{
 uword Zone::TryAllocate(int64_t size){
   return RawObject::TryAllocateIn(this, size);
 }

 void Zone::VisitObjectPointers(RawObjectVisitor* vis) const{
   return IteratePointers<Zone, ZoneIterator>(this, vis);
 }

 void Zone::VisitObjectPointers(RawObjectVisitorFunction vis) const{
   return IteratePointers<Zone, ZoneIterator>(this, vis);
 }

 void Zone::VisitMarkedObjectPointers(RawObjectVisitor* vis) const{
   return IterateMarkedPointers<Zone, ZoneIterator>(this, vis);
 }

 void Zone::VisitMarkedObjectPointers(RawObjectVisitorFunction vis) const{
   return IterateMarkedPointers<Zone, ZoneIterator>(this, vis);
 }
}