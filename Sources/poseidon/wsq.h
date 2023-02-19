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
   std::atomic<uword> top_;
   std::atomic<uword> bottom_;
   std::vector<T> tasks_;
  public:
   explicit WorkStealingQueue(uword capacity = 1024):
     top_(0),
     bottom_(0),
     tasks_(capacity, 0) { }
   ~WorkStealingQueue() = default;

   uword size() const{
     return bottom_.load(std::memory_order_seq_cst) -
            top_.load(std::memory_order_seq_cst);
   }

   bool IsEmpty() const {
     return size() == 0;
   }

   bool Push(T item) {
     auto bottom = bottom_.load(std::memory_order_acquire);
     if(bottom < tasks_.size()) {
       tasks_[bottom % tasks_.size()] = item;
       std::atomic_thread_fence(std::memory_order_release);
       bottom_.store(bottom + 1, std::memory_order_release);
       return true;
     }
     return false;
   }

   T Pop(){
     auto bottom = bottom_.load(std::memory_order_acquire);
     if(bottom > 0) {
       bottom -= 1;
       bottom_.store(bottom, std::memory_order_release);
     }

     std::atomic_thread_fence(std::memory_order_release);
     auto top = top_.load(std::memory_order_acquire);
     if(top <= bottom) {
       auto task = tasks_[bottom % tasks_.size()];
       if(top == bottom) {
         auto expectedTop = top;
         const auto nextTop = top + 1;
         auto desiredTop = nextTop;
         if(!top_.compare_exchange_strong(expectedTop, desiredTop, std::memory_order_acq_rel))
           task = (T)0;
         bottom_.store(nextTop, std::memory_order_release);
       }
       return task;
     }

     bottom_.store(top, std::memory_order_release);
     return (T)0;
   }

   T Steal() {
     auto top = top_.load(std::memory_order_acquire);
     std::atomic_thread_fence(std::memory_order_acquire);
     auto bottom = bottom_.load(std::memory_order_acquire);
     if(top < bottom) {
       auto task = tasks_[top % tasks_.size()];
       const auto nextTop = top + 1;
       auto desiredTop = nextTop;
       if(!top_.compare_exchange_weak(top, desiredTop, std::memory_order_acq_rel)) {
         return (T)0;
       } else {
         return task;
       }
     } else {
       return (T)0;
     }
   }
 };
}

#endif //POSEIDON_WSQ_H