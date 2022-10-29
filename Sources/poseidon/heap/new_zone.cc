#include "poseidon/heap/new_zone.h"
#include "poseidon/heap/new_page.h"
#include "poseidon/collector/collector.h"

namespace poseidon{
#define UNALLOCATED 0 //TODO: cleanup

  void NewZone::MarkAllIntersectedBy(const Region& region){
    NewZonePageIterator iter(this);
    while(iter.HasNext()) {
      auto next = iter.Next();
      if(next->Contains(region))
        MarkPage(GetPageIndex(next));
    }
  }

 NewZone* NewZone::New(const poseidon::MemoryRegion& region){
   const auto total_size = region.GetSize() - NewZone::GetHeaderSize();
   const auto semi_size = total_size / 2;
   return new ((void*)region.GetStartingAddressPointer())NewZone(total_size, semi_size);
 }

 bool NewZone::InitializePages(const MemoryRegion& region) {
   const auto page_size = GetNewPageSize();
   const auto num_pages = CalculateNumberOfPages(region, page_size);
   return false;
//TODO:
//   pages_ = new NewPage[num_pages];
//   for(num_pages_ = 0; num_pages_ < num_pages; num_pages_++)
//     pages_[num_pages_] = NewPage(num_pages_, MemoryRegion::Subregion(region, num_pages_ * page_size, page_size));
//   return num_pages_ == num_pages;
 }

 bool NewZone::VisitPages(NewPageVisitor* vis){
   NewZonePageIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(!vis->VisitNewPage(next))
       return false;
   }
   return true;
 }

 bool NewZone::VisitMarkedPages(NewPageVisitor* vis){
   NewZonePageIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(IsPageMarked(GetPageIndex(next)) && !vis->VisitNewPage(next))
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
   if(size <= 0 || size >= GetNewPageSize()) //TODO: cleanup
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
   MarkAllIntersectedBy(*ptr);
   return ptr->GetStartingAddress();
 }
}