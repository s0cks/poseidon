#include "poseidon/heap/new_zone.h"
#include "poseidon/collector/collector.h"

namespace poseidon{
 uword NewZone::TryAllocate(int64_t size){
   auto total_size = static_cast<int64_t>(sizeof(RawObject)) + size;
   if((GetCurrentAddress() + total_size) >= (fromspace() + tospace())){
     PSDN_CANT_ALLOCATE(ERROR, total_size, (*this));
     Collector::MinorCollection();
   }

   if((GetCurrentAddress() + total_size) >= (fromspace() + tospace())){
     PSDN_CANT_ALLOCATE(FATAL, total_size, (*this));
   }

   auto ptr = new (GetCurrentAddressPointer())RawObject(ObjectTag::NewWithSize(size));
   current_ += total_size;
   return ptr->GetAddress();
 }
}