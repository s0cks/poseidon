#ifndef POSEIDON_FREELIST_H
#define POSEIDON_FREELIST_H

#include <cstdlib>
#include <utility>
#include <glog/logging.h>

#include "poseidon/utils.h"
#include "poseidon/bitset.h"
#include "poseidon/raw_object.h"
#include "poseidon/platform/platform.h"

namespace poseidon{ //TODO: atomic support?
 class FreeList{
  public:
   friend class ParallelSweeperTask;//TODO: remove
  public:
   struct Node{
     Node* previous;
     Node* next;

     uword start;
     int64_t size;

     Node() = default;
     Node(uword starting_address, int64_t total_size):
       previous(nullptr),
       next(nullptr),
       start(starting_address),
       size(total_size){
     }
     Node(const Node& rhs) = default;
     ~Node() = default;

     Node& operator=(const Node& rhs) = default;

     friend std::ostream& operator<<(std::ostream& stream, const Node& val){
       return stream << "Node(start=" << ((void*) val.start) << ", size=" << Bytes(val.size) << ")";
     }
   };
  private:
   std::mutex mutex_;//TODO: remove mutex?
   Node* list_;
   RelaxedAtomic<int64_t> free_bytes_;

   Node* FindBestFit(int64_t size);
   void Remove(Node* node);
   void Add(uword address, int64_t size);
  public:
   FreeList() = delete;
   FreeList(uword start, int64_t size):
    mutex_(),
    list_(new Node(start, size)),
    free_bytes_(size){
   }
   FreeList(const FreeList& rhs) = delete;
   ~FreeList(){
     delete list_;
   }

   int64_t GetTotalBytesFree() const{
     return (int64_t)free_bytes_;
   }

   uword TryAllocate(int64_t size);
   void VisitFreeList(const std::function<bool(Node*)>& vis) const;
 };

 void PrintFreeList(FreeList* free_list);
}

#endif//POSEIDON_FREELIST_H