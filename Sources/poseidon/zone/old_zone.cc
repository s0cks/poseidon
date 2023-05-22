#include "poseidon/object.h"
#include "poseidon/heap/page_marker.h"
#include "old_zone.h"
#include "poseidon/collector/collector.h"

namespace poseidon{
 Pointer* OldZone::TryAllocatePointer(const word size) { //TODO: cleanup
   if(size < GetMinimumObjectSize() || size > GetMaximumObjectSize()) {
     LOG(WARNING) << "cannot allocate " << Bytes(size) << " in " << (*this);
     return UNALLOCATED;
   }

   DLOG(INFO) << "allocating " << Bytes(size) << " in " << (*this);
   Pointer* new_ptr = nullptr;
   if((new_ptr = free_list_.TryAllocatePointer(size)) == UNALLOCATED) {
     PSDN_CANT_ALLOCATE(ERROR, size, (*this));
     Collector::MajorCollection();
     if((new_ptr = free_list_.TryAllocatePointer(size)) == UNALLOCATED) {
       PSDN_CANT_ALLOCATE(FATAL, size, (*this));
     }
   }
   //TODO: mark all pages intersected by new_ptr
   return new_ptr;
 }

 uword OldZone::TryAllocateBytes(word size) {
   auto new_ptr = TryAllocatePointer(size);
   if(new_ptr == UNALLOCATED)
     return UNALLOCATED;
   return new_ptr->GetObjectPointerAddress();
 }

 uword OldZone::TryAllocateClassBytes(Class* cls) {
   return TryAllocateBytes(cls->GetAllocationSize());
 }
}