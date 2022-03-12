#ifndef POSEIDON_WSQ_H
#define POSEIDON_WSQ_H

#include <atomic>
#include "poseidon/utils.h"
#include "poseidon/platform.h"

namespace poseidon{
 template<class T>
 class WorkStealingQueue{
  private:
   uint64_t capacity_;
   std::vector<T> data_;
   std::atomic<uint64_t> top_;
   std::atomic<uint64_t> bottom_;
  public:
   explicit WorkStealingQueue(int64_t capacity):
     capacity_(0),
     data_(),
     top_(0),
     bottom_(0){
     if(capacity > 0){
       uint64_t new_capacity = RoundUpPowTwo(capacity);
       capacity_ = new_capacity;
       data_.resize(new_capacity, 0);
     }
   }
   ~WorkStealingQueue() = default;

   bool Push(const T& value){
     uint64_t bottom = bottom_.load(std::memory_order_acquire);
     if(bottom < data_.size()){
       data_[bottom % data_.size()] = value;
       std::atomic_thread_fence(std::memory_order_release);
       bottom_.store(bottom+1, std::memory_order_release);
       return true;
     }
     return false;
   }

   T Pop(){
     uint64_t bottom = bottom_.load(std::memory_order_acquire);
     if(bottom > 0){
       bottom -= 1;
       bottom_.store(bottom, std::memory_order_release);
     }

     std::atomic_thread_fence(std::memory_order_release);
     uint64_t top = top_.load(std::memory_order_acquire);
     if(top <= bottom){
       auto next = data_[bottom % data_.size()];
       if(top == bottom){
         uint64_t expected_top = top;
         uint64_t next_top = top + 1;
         uint64_t desired_top = next_top;
         if(!top_.compare_exchange_strong(expected_top, desired_top, std::memory_order_acq_rel)){
           next = (T)nullptr;
         }
         bottom_.store(next_top, std::memory_order_release);
       }
       return next;
     }
     bottom_.store(top, std::memory_order_release);
     return (T)nullptr;
   }

   T Steal(){
     uint64_t top = top_.load(std::memory_order_acquire);
     std::atomic_thread_fence(std::memory_order_acquire);
     uint64_t bottom = bottom_.load(std::memory_order_acquire);
     if(top < bottom){
       auto next = data_[top % data_.size()];
       uint64_t next_top = top+1;
       uint64_t desired_top = next_top;
       if(!top_.compare_exchange_weak(top, desired_top, std::memory_order_acq_rel)){
         return (T)nullptr;
       }
       return next;
     }
     return (T)nullptr;
   }

   uint64_t length() const{
     return bottom_.load(std::memory_order_seq_cst)-top_.load(std::memory_order_seq_cst);
   }

   bool empty() const{
     return length() == 0;
   }

   uint64_t capacity() const{
     return capacity_;
   }
 };
}

#endif //POSEIDON_WSQ_H