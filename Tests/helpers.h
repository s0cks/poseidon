#ifndef POSEIDON_HELPERS_H
#define POSEIDON_HELPERS_H

#include <gtest/gtest.h>
#include <glog/logging.h>

#include "poseidon/heap/heap.h"

namespace poseidon{
 static inline uword
 FailAllocation(int64_t size){
   DLOG(ERROR) << "allocating new object (" << Bytes(size) << ") failed.";
   return 0;
 }

 static inline uword
 SystemAlloc(int64_t size){
   return (uword)malloc(size);
 }

 static inline uword
 SystemAllocNewObject(int64_t size){
   DLOG(INFO) << "allocating new object (" << Bytes(size) << ") using malloc.";
   auto total_size = sizeof(RawObject) + size;
   auto val = (RawObject*)SystemAlloc(size);
   if(!val){
     LOG(FATAL) << "failed to allocate new object (" << Bytes(size) << ") using system [malloc].";
     return 0;
   }

   memset((void*)val, 0, total_size);
   val->SetPointerSize(size);
   return val->GetAddress();
 }
}

#endif //POSEIDON_HELPERS_H
