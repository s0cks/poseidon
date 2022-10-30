#include <glog/logging.h>
#include "poseidon/heap/heap.h"
#include "poseidon/collector/collector.h"

namespace poseidon{
#ifndef UNALLOCATED
#define UNALLOCATED 0 //TODO: cleanup
#endif // UNALLOCATED

 Heap* Heap::From(const MemoryRegion& region){
   if(region.GetStartingAddress() <= 0 || region.GetSize() <= 0)
     return nullptr;

   auto heap = new (region.GetStartingAddressPointer())Heap(region);
   return heap;
 }

 pthread_key_t Heap::kThreadKey = PTHREAD_KEYS_MAX;

 uword Heap::AllocateNewObject(int64_t size){
   RawObject* val = nullptr;
   if((val = (RawObject*)new_zone()->TryAllocate(size)) != nullptr)
     goto finish_allocation;

   DLOG(WARNING) << "couldn't allocate new object of " << Bytes(size) << ".";
   Collector::MinorCollection();

   if((val = (RawObject*)new_zone()->TryAllocate(size)) != nullptr)
     goto finish_allocation;

   LOG(FATAL) << "cannot allocate new object of " << Bytes(size) << "!";
   return 0;

finish_allocation:
   val->SetNewBit();
   return val->GetStartingAddress();
 }

 uword Heap::AllocateOldObject(int64_t size){
   RawObject* val = nullptr;

   // 1. Try Allocation
   if((val = (RawObject*)old_zone()->TryAllocate(size)) != nullptr)
     goto finish_allocation;

   // 2. Try Major Collection
   Collector::MajorCollection();

   // 3. Try Allocation Again
   if((val = (RawObject*)old_zone()->TryAllocate(size)) != nullptr)
     goto finish_allocation;

   // 4. Try Pages w/ Grow
   {
     //TODO: allocate using pages, with growth
   }

   // 5. Crash
   LOG(FATAL) << "cannot allocate " << Bytes(size) << " in heap.";
   return 0;

finish_allocation:
   val->SetOldBit();
   return val->GetStartingAddress();
 }

 uword Heap::AllocateLargeObject(int64_t size){
   return AllocateOldObject(size);//TODO: refactor
 }

 uword Heap::TryAllocate(int64_t size){
   if(size < kWordSize || size > GetOldPageSize())
     return UNALLOCATED;

   if(size < GetNewPageSize()) {
     uword address = UNALLOCATED;
     if((address = new_zone()->TryAllocate(size)) != UNALLOCATED)
       return address;
     PSDN_CANT_ALLOCATE(FATAL, size, (*this));
     return UNALLOCATED;
   }

   if(size >= GetNewPageSize()) {
     uword address = UNALLOCATED;
     if((address = old_zone()->TryAllocate(size)) != UNALLOCATED)
       return address;

     PSDN_CANT_ALLOCATE(FATAL, size, (*this));
     return UNALLOCATED;
   }

   PSDN_CANT_ALLOCATE(FATAL, size, (*this));
   return UNALLOCATED;
 }
}