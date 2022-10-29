#include "poseidon/heap/old_zone.h"

namespace poseidon{
#define UNALLOCATED 0 //TODO: cleanup

 OldZone* OldZone::From(const MemoryRegion& region){
   //TODO: check region size
   return new ((void*)region.GetStartingAddressPointer())OldZone(region.GetStartingAddress() + GetHeaderSize(), region.GetSize() - GetHeaderSize(), GetNumberOfOldPages());
 }

 bool OldZone::VisitPages(OldPageVisitor* vis){
   NOT_IMPLEMENTED(FATAL); //TODO: implement
   return false;
 }

 bool OldZone::VisitMarkedPages(OldPageVisitor* vis){
   NOT_IMPLEMENTED(FATAL); //TODO: implement
   return false;
 }

 bool OldZone::VisitPointers(RawObjectVisitor* vis){
   OldZoneIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(!vis->Visit(next))
       return false;
   }
   return true;
 }

 bool OldZone::VisitMarkedPointers(poseidon::RawObjectVisitor* vis){
   OldZoneIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(next->IsMarked() && !vis->Visit(next))
       return false;
   }
   return true;
 }

 bool OldZone::InitializePages(const MemoryRegion& region) {
   DLOG(INFO) << "initializing pages";
   const auto page_size = GetOldPageSize();
   const auto num_pages = CalculateNumberOfPages(region, page_size);
   table_ = BitSet(num_pages);
   return false;
//TODO: implement
//   pages_ = new OldPage[num_pages];
//   for(num_pages_ = 0; num_pages_ < num_pages; num_pages_++) {
//     const auto page_offset = num_pages_ * page_size;
//     pages_[num_pages_] = OldPage(num_pages_, MemoryRegion::Subregion(region, page_offset, page_size));
//   }
//   return num_pages_ == num_pages;
 }

 uword OldZone::TryAllocate(const ObjectSize& size) { //TODO: cleanup
   if(size <= 0 || size >= GetSize()) {
     LOG(WARNING) << "cannot allocate " << Bytes(size) << " in " << (*this);
     return UNALLOCATED;
   }

   auto ptr = (void*)free_list()->TryAllocate(size);
   auto val = new (ptr)RawObject();
   val->SetOldBit();
   val->SetPointerSize(size);
   MarkAllIntersectedBy((*val));
   return val->GetStartingAddress();
 }
}