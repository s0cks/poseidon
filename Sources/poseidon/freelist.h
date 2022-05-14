#ifndef POSEIDON_FREELIST_H
#define POSEIDON_FREELIST_H

#include <cstdlib>
#include <utility>
#include <glog/logging.h>

#include "poseidon/utils.h"
#include "poseidon/bitset.h"
#include "poseidon/platform.h"
#include "poseidon/raw_object.h"

namespace poseidon{ //TODO: atomic support?
 class FreeListObject{

 };

 class FreeList{
  public:
   FreeList() = default;
   ~FreeList() = default;

   uword TryAllocate(int64_t size){
     NOT_IMPLEMENTED(ERROR);//TODO: implement
     return 0;
   }

   void PrintFreeList(){//TODO: remove
     NOT_IMPLEMENTED(ERROR);
   }
 };
}

#endif//POSEIDON_FREELIST_H