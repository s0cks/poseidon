#include <glog/logging.h>

#include "poseidon/semispace.h"

namespace poseidon{
 uword Semispace::Allocate(int64_t size){
   auto total_size = static_cast<int64_t>(sizeof(RawObject)) + size;
   if(!Contains(current_ + total_size)){
     DLOG(WARNING) << "cannot allocate object of size " << HumanReadableSize(size) << " in space.";
     return 0;
   }

   auto next = (void*)current_;
   current_ += total_size;
   auto ptr = new (next)RawObject();
   ptr->SetPointerSize(size);
   return ptr->GetAddress();
 }
}