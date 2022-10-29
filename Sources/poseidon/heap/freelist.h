#ifndef POSEIDON_FREELIST_H
#define POSEIDON_FREELIST_H

#include <cstdlib>
#include <utility>
#include <glog/logging.h>

#include "poseidon/flags.h"
#include "poseidon/utils.h"
#include "poseidon/bitset.h"
#include "poseidon/raw_object.h"
#include "poseidon/platform/platform.h"
#include "poseidon/heap/section.h"
#include "poseidon/heap/free_object.h"

namespace poseidon{ //TODO: atomic support?
 class FreeList {
   friend class Sweeper;
   friend class OldZone;

   friend class FreeListTest;
   friend class SerialSweeperTest;
  protected:
   uword start_;
   int64_t total_size_;

   FreeObject** buckets_;
   int64_t num_nodes_;

   static inline int GetBucketIndexFor(const ObjectSize& size){
     return static_cast<int>(size % GetNumberOfFreeListBuckets());
   }

   static inline void
   InsertAfter(FreeObject* node, const uword start, const ObjectSize& size) {
     auto ptr = FreeObject::From(MemoryRegion(start, size));
     ptr->SetNextAddress(node->GetNextAddress());
     node->SetNextAddress(ptr->GetAddress());
   }
  public:
   FreeList() = default;
   explicit FreeList(const MemoryRegion& region):
    buckets_(new FreeObject*[GetNumberOfFreeListBuckets()]),
    num_nodes_(0),
    start_(region.GetStartingAddress()),
    total_size_(region.GetSize()) {
     for(auto idx = 0; idx < GetNumberOfFreeListBuckets(); idx++)
       buckets_[idx] = nullptr;
     Insert(region.GetStartingAddress(), region.GetSize());
   }
   FreeList(const FreeList& rhs) = default;
   ~FreeList() = default;

   int64_t GetNumberOfNodes() const {
     return num_nodes_;
   }

   virtual FreeObject* FindBestFit(const ObjectSize& size); //TODO: rename
   virtual void Insert(uword start, ObjectSize size);
   virtual bool Remove(uword start, ObjectSize size);

   virtual uword TryAllocate(const ObjectSize& size);
 };
}

#endif//POSEIDON_FREELIST_H