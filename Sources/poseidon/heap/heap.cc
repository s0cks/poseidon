#include <glog/logging.h>

#include "poseidon/heap/heap.h"
#include "poseidon/object.h"
#include "poseidon/collector/collector.h"

namespace poseidon{
 Heap* Heap::From(const MemoryRegion& region){
   if(region.GetStartingAddress() <= 0 || region.GetSize() <= 0)
     return nullptr;
   return new Heap(region);
 }

 pthread_key_t Heap::kThreadKey = PTHREAD_KEYS_MAX;

 uword Heap::AllocateNewObject(int64_t size){
   Pointer* val = nullptr;
   if((val = (Pointer*)new_zone()->TryAllocateBytes(size)) != nullptr)
     goto finish_allocation;

   DLOG(WARNING) << "couldn't allocate new object of " << Bytes(size) << ".";
   Collector::MinorCollection();

   if((val = (Pointer*)new_zone()->TryAllocateBytes(size)) != nullptr)
     goto finish_allocation;

   LOG(FATAL) << "cannot allocate new object of " << Bytes(size) << "!";
   return 0;

finish_allocation:
   //TODO: val->SetNewBit();
   return val->GetStartingAddress();
 }

 uword Heap::AllocateOldObject(int64_t size){
   Pointer* val = nullptr;

   // 1. Try Allocation
   if((val = (Pointer*)old_zone()->TryAllocateBytes(size)) != nullptr)
     goto finish_allocation;

   // 2. Try Major Collection
   Collector::MajorCollection();

   // 3. Try Allocation Again
   if((val = (Pointer*)old_zone()->TryAllocateBytes(size)) != nullptr)
     goto finish_allocation;

   // 4. Try Pages w/ Grow
   {
     //TODO: allocate using pages, with growth
   }

   // 5. Crash
   LOG(FATAL) << "cannot allocate " << Bytes(size) << " in heap.";
   return 0;

finish_allocation:
   //TODO: val->SetOldBit();
   return val->GetStartingAddress();
 }

 uword Heap::AllocateLargeObject(int64_t size){
   return AllocateOldObject(size);//TODO: refactor
 }

 Pointer* Heap::TryAllocatePointer(const word size) {
   if(size < kWordSize || size > flags::GetOldPageSize())
     return UNALLOCATED;

   if(size < flags::GetNewPageSize()) {
     Pointer* new_ptr = nullptr;
     if((new_ptr = new_zone()->TryAllocatePointer(size)) != UNALLOCATED)
       return new_ptr;
     PSDN_CANT_ALLOCATE(FATAL, size, (*this));
     return UNALLOCATED;
   }

   if(size >= flags::GetNewPageSize()) {
     Pointer* new_ptr = nullptr;
     if((new_ptr = old_zone()->TryAllocatePointer(size)) != UNALLOCATED)
       return new_ptr;

     PSDN_CANT_ALLOCATE(FATAL, size, (*this));
     return UNALLOCATED;
   }

   PSDN_CANT_ALLOCATE(FATAL, size, (*this));
   return UNALLOCATED;
 }

 uword Heap::TryAllocateBytes(word size) {
   auto new_ptr = TryAllocatePointer(size);
   if(new_ptr == UNALLOCATED)
     return UNALLOCATED;
   return new_ptr->GetObjectPointerAddress();
 }

 uword Heap::TryAllocateClassBytes(Class* cls) {
   return TryAllocateBytes(cls->GetAllocationSize());
 }
}