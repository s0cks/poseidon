#include "poseidon/platform/memory_region.h"

namespace poseidon{
 MemoryRegion::MemoryRegion(const MemoryRegion* parent, int64_t offset, int64_t size)://TODO: Refactor
   start_(0),
   size_(0){
   if(size >= parent->size()){
     LOG(WARNING) << "cannot allocate MemoryRegion of " << Bytes(size) << ", size is larger than parent.";
     return;
   }

   auto start = parent->GetStartingAddress() + offset;
   if(!parent->Contains(start)){
     DLOG(WARNING) << "cannot allocate MemoryRegion of " << Bytes(size) << " at offset " << offset << ", parent doesn't contain starting address: " << ((void*)start);
     return;
   }

#ifdef PSDN_DEBUG
   auto end = start + size;
   assert(parent->Contains(end));
#endif//PSDN_DEBUG
   start_ = start;
   size_ = size;
 }
}