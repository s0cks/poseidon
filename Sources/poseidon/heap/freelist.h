#ifndef POSEIDON_FREELIST_H
#define POSEIDON_FREELIST_H

#include <cstdlib>
#include <utility>
#include <glog/logging.h>

#include "poseidon/utils.h"
#include "poseidon/bitset.h"
#include "poseidon/raw_object.h"
#include "poseidon/platform/platform.h"
#include "poseidon/heap/section.h"
#include "poseidon/heap/freelist_node.h"

namespace poseidon{ //TODO: atomic support?
 class FreeList {
   friend class Sweeper;
   friend class OldZone;
  public:
   class FreeListIterator {
    protected:
     FreeList* free_list_;

     inline FreeList* free_list() const {
       return free_list_;
     }
    public:
     explicit FreeListIterator(FreeList* free_list):
      free_list_(free_list) {
     }
     virtual ~FreeListIterator() = default;

     virtual bool HasNext() const {
       NOT_IMPLEMENTED(ERROR); //TODO: implement
       return false;
     }

     virtual FreeListNode* Next() {
       NOT_IMPLEMENTED(ERROR); //TODO: implement
       return nullptr;
     }
   };
  protected:
   virtual uword TryAllocate(ObjectSize size);
  public:
   FreeList() = default;
   explicit FreeList(const MemoryRegion& region):
    FreeList() {
   }
   FreeList(const FreeList& rhs) = default;
   ~FreeList() = default;

   virtual bool Visit(FreeListNodeVisitor* vis);

   FreeList& operator=(const FreeList& rhs) = default;
 };
}

#endif//POSEIDON_FREELIST_H