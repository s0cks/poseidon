#include "poseidon/zone.h"
#include "poseidon/heap.h"

namespace poseidon{
 void NewZone::VisitPages(const std::function<bool(NewPage*)>& vis) const{
   auto current = Zone::GetStartingAddress();
   while(current <= Zone::GetEndingAddress()){
     NewPage page(NewPage::GetIndexFromPageDimensions(Zone::GetStartingAddress(), current, kNewZonePageSize), current, kNewZonePageSize);
     if(!vis(&page))
       return;
     current += page.GetSize();
   }
 }
}