#ifndef POSEIDON_FREELIST_H
#define POSEIDON_FREELIST_H

#include <cstdlib>
#include <functional>
#include <glog/logging.h>

#include "poseidon/utils.h"
#include "poseidon/bitset.h"
#include "poseidon/platform.h"

namespace poseidon{
 class FreeList{
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
       return stream << "Node(start=" << ((void*)val.start) << ", size=" << Bytes(val.size) << ")";
     }
   };
  private:
   Node* list_;

   Node* FindBestFit(int64_t size){
     auto current = list_;
     while(current != nullptr && current->size < size)
       current = current->next;
     return current;
   }

   static inline bool
   CanSplit(Node* node, int64_t size){
     return (node->size - size) >= kWordSize;
   }

   static inline Node*
   Split(Node* node, int64_t size){
     auto new_size = node->size - size;
     auto new_start = node->start + size;
     return new Node(new_start, new_size);
   }

   void Remove(Node* node){
     if(node == list_){
       list_ = list_->next;
       return;
     }
     auto previous = node->previous;
     auto next = node->next;
     if(previous)
       previous->next = next;
     if(next)
       next->previous = previous;
   }

   static inline void
   InsertAfter(Node* previous, Node* node){
     node->previous = previous;
     node->next = previous->next;
     node->previous->next = node;
     if(node->next)
       node->next->previous = node;
   }

   static inline void
   Append(Node* tail, Node* node){
     node->previous = tail;
     tail->next = node;
   }

   static inline void
   InsertBefore(Node* next, Node* node){
     return InsertAfter(next->previous, node);
   }
  public:
   FreeList():
    list_(nullptr){
   }
   FreeList(uword start, int64_t size):
    list_(new Node(start, size)){
   }
   ~FreeList(){
     delete list_;
   }

   uword TryAllocate(int64_t size){
     if(size < kWordSize)//TODO: Fixme
       return 0;
     //TODO: align size
     auto best_fit = FindBestFit(size);
     if(!best_fit){
       return 0;
     }

     if(CanSplit(best_fit, size)){
       auto new_node = Split(best_fit, size);
       InsertAfter(list_, new_node);
     }
     Remove(best_fit);
     return best_fit->start;
   }

   void VisitFreeList(const std::function<bool(Node*)>& vis) const{
     auto node = list_;
     while(node != nullptr){
       if(!vis(node))
         return;
       node = node->next;
     }
   }

   void PrintFreeList(){
     DLOG(INFO) << "free list:";
     VisitFreeList([&](Node* node){
       DLOG(INFO) << " - " << (*node);
       return true;
     });
   }
 };
}

#endif//POSEIDON_FREELIST_H
