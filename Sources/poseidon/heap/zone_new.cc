#include "poseidon/allocator.h"
#include "poseidon/heap/zone_new.h"

namespace poseidon{
 uword NewZone::TryAllocate(int64_t size){
   auto total_size = size + sizeof(RawObject);
   if((current_ + total_size) >= (fromspace_ + tospace_))
     Allocator::MinorCollection();

   if((current_ + total_size) >= (fromspace_ + tospace_)){
     LOG(FATAL) << "insufficient memory.";
     return 0;
   }

   auto ptr = current_;
   current_ = current_ + total_size;
   new ((void*)ptr)RawObject(size);
   return ptr;
 }
}