#include <glog/logging.h>


#include "poseidon/heap.h"
#include "poseidon/utils.h"
#include "poseidon/allocator.h"
#include "poseidon/raw_object.h"

namespace poseidon{
 pthread_key_t Heap::kThreadKey = PTHREAD_KEYS_MAX;

 uword Heap::AllocateNewObject(int64_t size){
   RawObject* val = nullptr;
   if((val = (RawObject*)new_zone().TryAllocate(size)) != nullptr)
     goto finish_allocation;

   DLOG(WARNING) << "couldn't allocate new object of " << Bytes(size) << ".";
   Allocator::MinorCollection();

   if((val = (RawObject*)new_zone().TryAllocate(size)) != nullptr)
     goto finish_allocation;

   LOG(FATAL) << "cannot allocate new object of " << Bytes(size) << "!";
   return 0;

finish_allocation:
   val->SetNewBit();
   return val->GetAddress();
 }

 uword Heap::AllocateOldObject(int64_t size){
   RawObject* val = nullptr;

   // 1. Try Allocation
   if((val = (RawObject*)old_zone().TryAllocate(size)) != nullptr)
     goto finish_allocation;

   // 2. Try Major Collection
   //TODO: free memory from old_zone

   // 3. Try Allocation Again
   if((val = (RawObject*)old_zone().TryAllocate(size)) != nullptr)
     goto finish_allocation;

   // 4. Try Pages w/ Grow
   {
     //TODO: allocate using pages, with growth
   }

   // 5. Crash
   LOG(FATAL) << "cannot allocate " << Bytes(size) << " in heap.";
   return 0;

finish_allocation:
   val->SetNewBit();
   return val->GetAddress();
 }

 uword Heap::AllocateLargeObject(int64_t size){
   return AllocateOldObject(size);//TODO: refactor
 }

 uword Heap::TryAllocate(int64_t size){
   if(size < kWordSize)
     size = kWordSize;

   if(size >= GetLargeObjectSize()){
     DLOG(INFO) << "allocating large object of " << Bytes(size);
     return AllocateLargeObject(size);
   }
   return AllocateNewObject(size);
 }
}