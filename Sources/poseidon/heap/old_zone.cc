#include "poseidon/heap/old_zone.h"
#include "poseidon/heap/page_marker.h"
#include "poseidon/collector/collector.h"

namespace poseidon{
#define UNALLOCATED 0 //TODO: cleanup

 bool OldZone::MarkAllIntersectedBy(const poseidon::Region& region){
   return PageMarker<OldZone>::MarkAllIntersectedBy(this, region);
 }

 OldZone* OldZone::From(const MemoryRegion& region){
   return new OldZone(region.GetStartingAddress(), region.GetSize());
 }

 bool OldZone::VisitPointers(RawObjectVisitor* vis) {
   OldZoneIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(!vis->Visit(next))
       return false;
   }
   return true;
 }

 bool OldZone::VisitPointers(std::function<bool(Pointer*)>& function) {
   OldZoneIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(!function(next))
       return false;
   }
   return true;
 }

 bool OldZone::VisitMarkedPointers(RawObjectVisitor* vis) {
   OldZoneIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(next->IsMarked() && !vis->Visit(next))
       return false;
   }
   return true;
 }

 bool OldZone::VisitMarkedPointers(std::function<bool(Pointer*)>& function) {
   OldZoneIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(next->IsMarked() && !function(next))
       return false;
   }
   return true;
 }

 uword OldZone::TryAllocateBytes(const ObjectSize size) { //TODO: cleanup
   if(size < GetMinimumObjectSize() || size > GetMaximumObjectSize()) {
     LOG(WARNING) << "cannot allocate " << Bytes(size) << " in " << (*this);
     return UNALLOCATED;
   }

   uword ptr_address = UNALLOCATED;
   if((ptr_address = free_list_.TryAllocate(size)) == UNALLOCATED) {
     PSDN_CANT_ALLOCATE(ERROR, size, (*this));
     Collector::MajorCollection();
     if((ptr_address = free_list_.TryAllocate(size)) == UNALLOCATED) {
       PSDN_CANT_ALLOCATE(FATAL, size, (*this));
     }
   }

   auto val = new ((void*) ptr_address)Pointer(PointerTag::Old(size));
   if(!MarkAllIntersectedBy((*val))) {
     PSDN_CANT_ALLOCATE(ERROR, size, (*this));
     //TODO: deallocate
     return UNALLOCATED;
   }
   memset((void*) val->GetObjectPointerAddress(), 0, val->GetSize());
   return val->GetObjectPointerAddress();
 }
}