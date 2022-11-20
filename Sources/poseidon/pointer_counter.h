#ifndef POSEIDON_POINTER_COUNTER_H
#define POSEIDON_POINTER_COUNTER_H

#include "poseidon/pointer.h"
#include "poseidon/relaxed_atomic.h"

namespace poseidon {
 class PointerCounter {
  private:
   RelaxedAtomic<int64_t> count_;
   RelaxedAtomic<int64_t> bytes_;
  public:
   PointerCounter() = default;
   PointerCounter(const PointerCounter& rhs) = default;
   ~PointerCounter() = default;

   int64_t count() const {
     return (int64_t)count_;
   }

   int64_t bytes() const {
     return (int64_t)bytes_;
   }

   PointerCounter& operator+=(const Pointer* ptr) {
     count_ += 1;
     bytes_ += ptr->GetSize();
     return *this;
   }

   PointerCounter& operator+=(const PointerCounter& rhs) {
     count_ += rhs.count();
     bytes_ += rhs.bytes();
     return *this;
   }

   PointerCounter& operator-=(const Pointer* ptr) {
     count_ -= 1;
     bytes_ -= ptr->GetSize();
     return *this;
   }

   PointerCounter& operator-=(const PointerCounter& rhs) {
     count_ -= rhs.count();
     bytes_ -= rhs.bytes();
     return *this;
   }

   PointerCounter& operator=(const PointerCounter& rhs) = default;
 };
}

#endif // POSEIDON_POINTER_COUNTER_H