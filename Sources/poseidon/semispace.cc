#include <glog/logging.h>
#include "poseidon/semispace.h"

namespace poseidon{
 RawObject* Semispace::AllocateRawObject(const uint64_t& size){
   uint64_t total_size = sizeof(RawObject) + size;
   if(!Contains(current_ + total_size)){
     DLOG(WARNING) << "cannot allocate object of size " << HumanReadableSize(size) << " in space.";
     return nullptr;
   }

   uword paddress = current_;
   current_ += total_size;
   void* ptr = (void*)paddress;
   memset(ptr, 0, total_size);
   auto raw = new (ptr)RawObject();
   raw->SetPointerSize(size);
   return raw;
 }
}