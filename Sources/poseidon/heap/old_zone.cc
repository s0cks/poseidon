#include "poseidon/heap/old_zone.h"

namespace poseidon{
 uword OldZone::TryAllocate(int64_t size){
   auto ptr = (void*)free_list()->TryAllocate(size);
   auto val = new (ptr)RawObject();
   val->SetOldBit();
   val->SetPointerSize(size);

   pages_.Mark(GetPageIndexFor(val->GetAddress()));

   return val->GetAddress();
 }

 void OldZone::VisitPages(const std::function<bool(OldPage*)>& vis) const{
   for(auto& page : pages_){
     if(!vis(&page))
       return;
   }
 }

 void OldZone::VisitMarkedPages(const std::function<bool(OldPage*)>& vis) const{
   for(auto& page : pages_){
     if(pages_.IsMarked(&page) && !vis(&page))
       return;
   }
 }
}