#include "poseidon/zone.h"

namespace poseidon{
 uword Zone::TryAllocate(int64_t size){
   auto total_size = static_cast<int64_t>(sizeof(RawObject)) + size;
   if(!Contains(current_ + total_size)){
     DLOG(WARNING) << "cannot allocate object of size " << Bytes(size) << " in space.";
     return 0;
   }

   auto next = (void*)current_;
   current_ += total_size;
   auto ptr = new (next)RawObject();
   ptr->SetPointerSize(size);
   return ptr->GetAddress();
 }

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