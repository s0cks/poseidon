#include "poseidon/heap/new_zone.h"
#include "poseidon/heap/new_page.h"
#include "poseidon/collector/collector.h"

namespace poseidon{
#define UNALLOCATED 0 //TODO: cleanup

 bool NewZone::InitializePages(const MemoryRegion& region) {
   const auto page_size = GetNewPageSize();
   const auto num_pages = CalculateNumberOfPages(region, page_size);
   table_ = BitSet(num_pages);
   pages_ = new NewPage[num_pages];
   for(num_pages_ = 0; num_pages_ < num_pages; num_pages_++)
     pages_[num_pages_] = NewPage(num_pages_, MemoryRegion::Subregion(region, num_pages_ * page_size, page_size));
   return num_pages_ == num_pages;
 }

 bool NewZone::VisitPages(PageVisitor* vis){
   NewZonePageIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(!vis->VisitPage(next))
       return false;
   }
   return true;
 }

 bool NewZone::VisitMarkedPages(PageVisitor* vis){
   NewZonePageIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(next->marked() && !vis->VisitPage(next))
       return false;
   }
   return true;
 }

 bool NewZone::VisitUnmarkedPages(PageVisitor* vis){
   NewZonePageIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(!next->marked() && !vis->VisitPage(next))
       return false;
   }
   return true;
 }

 bool NewZone::VisitPointers(RawObjectVisitor* vis){
   NewZoneIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(!vis->Visit(next))
       return false;
   }
   return true;
 }

 bool NewZone::VisitMarkedPointers(RawObjectVisitor* vis){
   NewZoneIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(next->IsMarked() && !vis->Visit(next))
       return false;
   }
   return true;
 }

 uword NewZone::TryAllocate(int64_t size){
   if(size <= 0 || size >= semisize())
     return UNALLOCATED;

   LOG(INFO) << "allocating " << Bytes(size) << " in " << (*this);
   auto total_size = static_cast<int64_t>(sizeof(RawObject)) + size;
   if((GetCurrentAddress() + total_size) >= (fromspace() + tospace())){
     PSDN_CANT_ALLOCATE(ERROR, total_size, (*this));
     Collector::MinorCollection();
   }

   if((GetCurrentAddress() + total_size) >= (fromspace() + tospace())){
     PSDN_CANT_ALLOCATE(FATAL, total_size, (*this));
   }

   auto ptr = new (GetCurrentAddressPointer())RawObject(ObjectTag::New(size));
   current_ += total_size;
   MarkAllIntersectedBy((*ptr));
   return ptr->GetStartingAddress();
 }
}