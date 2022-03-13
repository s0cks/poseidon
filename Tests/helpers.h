#ifndef POSEIDON_HELPERS_H
#define POSEIDON_HELPERS_H

#include <gtest/gtest.h>
#include <glog/logging.h>

#include "poseidon/heap.h"

namespace poseidon{
 static inline uword
 FailAllocation(uint64_t size){
   DLOG(ERROR) << "allocating new object (" << HumanReadableSize(size) << ") failed.";
   return 0;
 }

 static inline uword
 AllocateNewObjectUsingSystem(int64_t size){
   DLOG(INFO) << "allocating new object (" << HumanReadableSize(size) << ") using malloc.";
   auto total_size = sizeof(RawObject) + size;
   auto val = (RawObject*)malloc(total_size);
   memset((void*)val, 0, total_size);
   val->SetPointerSize(size);
   return val->GetAddress();
 }
}

#endif //POSEIDON_HELPERS_H
