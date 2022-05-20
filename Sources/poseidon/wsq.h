#ifndef POSEIDON_WSQ_H
#define POSEIDON_WSQ_H

#include <atomic>
#include <vector>
#include <optional>

#include "poseidon/utils.h"
#include "poseidon/platform/platform.h"

namespace poseidon{
 template<class T>
 class WorkStealingQueue{
  private:
   struct Item{
     int64_t C;
     int64_t M;
     std::atomic<T>* S;

     explicit Item(int64_t c):
       C(c),
       M(c - 1),
       S(new std::atomic<T>[static_cast<size_t>(C)]){
     }
     ~Item(){
       delete[] S;
     }

     int64_t capacity() const{
       return C;
     }

     void Push(int64_t i, T val){
       S[i & M].store(val, std::memory_order_relaxed);
     }

     T Pop(int64_t i){
       return S[i & M].load(std::memory_order_relaxed);
     }

     Item* Resize(int64_t bottom, int64_t top){
       auto ptr = new Item{2 * C};
       for(auto i = top; i != bottom; i++)
         ptr->Push(i, Pop(i));
       return ptr;
     }
   };
  private:
   std::atomic<int64_t> top_;
   std::atomic<int64_t> bottom_;
   std::atomic<Item*> items_;
   std::vector<Item*> garbage_;
  public:
   explicit WorkStealingQueue(int64_t capacity = 1024):
     top_(),
     bottom_(),
     items_(),
     garbage_(){
     auto cap = RoundUpPowTwo(capacity);
    top_.store(0, std::memory_order_relaxed);
    bottom_.store(0, std::memory_order_relaxed);
    items_.store(new Item{static_cast<int64_t>(cap)}, std::memory_order_relaxed);
    garbage_.reserve(32);
   }
   ~WorkStealingQueue(){
     for(auto& a : garbage_)
       delete a;
     delete items_.load();
   }

   bool empty() const{
     int64_t bottom = bottom_.load(std::memory_order_relaxed);
     int64_t top = top_.load(std::memory_order_relaxed);
     return bottom <= top;
   }

   int64_t size() const{
     int64_t bottom = bottom_.load(std::memory_order_relaxed);
     int64_t top = top_.load(std::memory_order_relaxed);
     return static_cast<int64_t>(bottom >= top ? bottom - top : 0);
   }

   int64_t capacity() const{
     return items_.load(std::memory_order_relaxed)->capacity();
   }

   void Push(T item){
     int64_t bottom = bottom_.load(std::memory_order_relaxed);
     int64_t top = top_.load(std::memory_order_acquire);
     auto items = items_.load(std::memory_order_relaxed);

     if((items->capacity() - 1) < (bottom - top)){
       auto tmp = items->Resize(bottom, top);
       garbage_.push_back(items);
       std::swap(items, tmp);
       items_.store(items, std::memory_order_relaxed);
     }

     items->Push(bottom, item);
     std::atomic_thread_fence(std::memory_order_release);
     bottom_.store(bottom + 1, std::memory_order_relaxed);
   }

   T Pop(){
     int64_t bottom = bottom_.load(std::memory_order_relaxed) - 1;
     auto items = items_.load(std::memory_order_relaxed);
     bottom_.store(bottom, std::memory_order_relaxed);
     std::atomic_thread_fence(std::memory_order_seq_cst);
     int64_t top = top_.load(std::memory_order_relaxed);

     T value = (T)0;
     if(top <= bottom){
       value = items->Pop(bottom);
       if(top == bottom){
         if(!top_.compare_exchange_strong(top, top + 1,
                                          std::memory_order_seq_cst,
                                          std::memory_order_relaxed)){
           value = (T)0;
         }
         bottom_.store(bottom + 1, std::memory_order_relaxed);
       }
     } else{
       bottom_.store(bottom + 1, std::memory_order_relaxed);
     }
     return value;
   }

   T Steal(){
     int64_t top = top_.load(std::memory_order_acquire);
     std::atomic_thread_fence(std::memory_order_seq_cst);
     int64_t bottom = bottom_.load(std::memory_order_acquire);

     T value = (T)0;
     if(top < bottom){
       auto items = items_.load(std::memory_order_consume);
       value = items->Pop(top);
       if(!top_.compare_exchange_strong(top, top + 1,
                                        std::memory_order_seq_cst,
                                        std::memory_order_relaxed)){
         return (T)0;
       }
     }
     return value;
   }
 };
}

#endif //POSEIDON_WSQ_H