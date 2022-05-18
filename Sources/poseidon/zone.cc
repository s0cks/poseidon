#include "poseidon/zone.h"
#include "poseidon/allocator.h"

namespace poseidon{
 void Zone::VisitObjectPointers(RawObjectVisitor* vis) const{
   ZoneIterator it(this);
   while(it.HasNext()){
     if(!vis->Visit(it.Next()))
       return;
   }
 }

 void Zone::VisitMarkedObjectPointers(RawObjectVisitor* vis) const{
   ZoneIterator it(this);
   while(it.HasNext()){
     if(!vis->Visit(it.Next()))
       return;
   }
 }

 void Zone::VisitObjectPointers(const std::function<bool(RawObject*)>& vis) const{
   ZoneIterator it(this);
   while(it.HasNext()){
     if(!vis(it.Next()))
       return;
   }
 }

 void Zone::VisitMarkedObjectPointers(const std::function<bool(RawObject*)>& vis) const{
   ZoneIterator it(this);
   while(it.HasNext()){
     if(!vis(it.Next()))
       return;
   }
 }
}