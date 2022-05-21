#include "poseidon/heap/new_zone.h"
#include "poseidon/collector/collector.h"

namespace poseidon{
 uword NewZone::TryAllocate(int64_t size){
   auto total_size = size + sizeof(RawObject);
   if((GetCurrentAddress() + total_size) >= (fromspace() + tospace()))
     Collector::MinorCollection();

   if((GetCurrentAddress() + total_size) >= (fromspace() + tospace())){
     LOG(FATAL) << "insufficient memory.";
     return 0;
   }

   auto ptr = new (GetCurrentAddressPointer())RawObject();
   current_ = current_ + total_size;
   ptr->SetNewBit();
   ptr->SetPointerSize(size);
   return ptr->GetAddress();
 }
}