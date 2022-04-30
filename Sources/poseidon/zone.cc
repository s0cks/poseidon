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

 uword NewZone::TryAllocate(int64_t size){
   auto total_size = size + sizeof(RawObject);
   if((current_ + total_size) >= (fromspace_ + tospace_))
     Allocator::MinorCollection();
   if((current_ + total_size) >= (fromspace_ + tospace_)){
     LOG(FATAL) << "insufficient memory.";
     return 0;
   }

   auto next = (void*)current_;
   current_ = current_ + total_size;
   auto ptr = new (next)RawObject();
   ptr->SetPointerSize(size);
   return ptr->GetAddress();
 }
}