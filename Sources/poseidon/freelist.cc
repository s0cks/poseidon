#include "poseidon/freelist.h"

namespace poseidon{
 static inline bool
 CanSplit(FreeList::Node* node, int64_t size){
   return (node->size - size) >= kWordSize;
 }

 static inline FreeList::Node*
 Split(FreeList::Node* node, int64_t size){
   auto new_size = node->size - size;
   auto new_start = node->start + size;
   return new FreeList::Node(new_start, new_size);
 }

 static inline void
 InsertAfter(FreeList::Node* previous, FreeList::Node* node){
   node->previous = previous;
   node->next = previous->next;
   node->previous->next = node;
   if(node->next)
     node->next->previous = node;
 }

 static inline void
 Append(FreeList::Node* tail, FreeList::Node* node){
   node->previous = tail;
   tail->next = node;
 }

 static inline void
 InsertBefore(FreeList::Node* next, FreeList::Node* node){
   return InsertAfter(next->previous, node);
 }

 FreeList::Node* FreeList::FindBestFit(int64_t size){
   std::lock_guard<std::mutex> guard(mutex_);
   auto current = list_;
   while(current != nullptr && current->size < size)
     current = current->next;
   return current;
 }

 void FreeList::Remove(Node* node){
   std::lock_guard<std::mutex> guard(mutex_);
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

 void FreeList::Add(uword address, int64_t size){
   std::lock_guard<std::mutex> guard(mutex_);
   auto node = list_;
   while(node->next != nullptr)
     node = node->next;
   Append(node, new Node(address, size));
   free_bytes_ += size;
 }

 uword FreeList::TryAllocate(int64_t size){
   if(size < kWordSize){
     DLOG(WARNING) << "object size " << Bytes(size) << " is smaller than a word, cannot allocate in FreeList.";
     return 0;
   }

   auto total_size = static_cast<int64_t>(sizeof(RawObject) + size);

   //TODO: align size
   auto best_fit = FindBestFit(total_size);
   if(!best_fit){
     DLOG(WARNING) << "couldn't find best fit in FreeList for object of " << Bytes(total_size) << ".";
     return 0;
   }

   if(CanSplit(best_fit, total_size)){
     std::lock_guard<std::mutex> guard(mutex_);
     auto new_node = Split(best_fit, total_size);
     InsertAfter(list_, new_node);
   }

   Remove(best_fit);
#ifdef PSDN_DEBUG
   memset((void*)best_fit->start, 0, best_fit->size);
#endif//PSDN_DEBUG
   free_bytes_ -= size;
   return best_fit->start;
 }

 void FreeList::VisitFreeList(const std::function<bool(Node*)>& vis) const{
   auto node = list_;
   while(node != nullptr){
     if(!vis(node))
       return;
     node = node->next;
   }
 }

 void PrintFreeList(FreeList* list){
   list->VisitFreeList([](FreeList::Node* node){
     DLOG(INFO) << " - " << (*node);
     return true;
   });
 }
}