#ifndef POSEIDON_RELAXED_ATOMIC_H
#define POSEIDON_RELAXED_ATOMIC_H

#include <atomic>

namespace poseidon{
 template<typename T>
 class RelaxedAtomic{
  private:
   std::atomic<T> val_;
  public:
   explicit RelaxedAtomic(const T& init):
     val_(init){
   }
   RelaxedAtomic(const RelaxedAtomic<T>& rhs):
    val_((T)rhs){
   }
   ~RelaxedAtomic() = default;

   T fetch_add(T arg, std::memory_order order=std::memory_order_relaxed){
     return val_.fetch_add(arg, order);
   }

   T fetch_sub(T arg, std::memory_order order=std::memory_order_relaxed){
     return val_.fetch_sub(arg, order);
   }

   T load(std::memory_order order=std::memory_order_relaxed) const{
     return val_.load(order);
   }

   void store(T arg, std::memory_order order=std::memory_order_relaxed){
     val_.store(arg, order);
   }

   bool compare_exchange_weak(T& expected, T desired, std::memory_order order=std::memory_order_relaxed){
     return val_.compare_exchange_weak(expected, desired, order, order);
   }

   bool compare_exchange_strong(T& expected, T desired, std::memory_order order=std::memory_order_relaxed){
     return val_.compare_exchange_strong(expected, desired, order, order);
   }

   explicit operator T() const{
     return load();
   }

   T operator=(T arg){ // NOLINT(misc-unconventional-assign-operator)
     store(arg);
     return arg;
   }

   T operator=(const RelaxedAtomic& arg){ // NOLINT(misc-unconventional-assign-operator)
     T loaded = arg;
     store(loaded);
     return loaded;
   }

   T operator+=(T arg){
     return fetch_add(arg) + arg;
   }

   T operator-=(T arg){
     return fetch_sub(arg) - arg;
   }

   friend bool operator==(const RelaxedAtomic<T>& lhs, const T& rhs){
     return ((T)lhs) == rhs;
   }

   friend bool operator!=(const RelaxedAtomic<T>& lhs, const T& rhs){
     return ((T)lhs) != rhs;
   }

   friend bool operator<(const RelaxedAtomic<T>& lhs, const T& rhs){
     return ((T)lhs) < rhs;
   }

   friend bool operator>(const RelaxedAtomic<T>& lhs, const T& rhs){
     return ((T)lhs) > rhs;
   }

   friend std::ostream& operator<<(std::ostream& stream, const RelaxedAtomic<T>& val){
     return stream << ((T)val);
   }
 };
}

#endif //POSEIDON_RELAXED_ATOMIC_H
