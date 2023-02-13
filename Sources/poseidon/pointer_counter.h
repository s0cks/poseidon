#ifndef POSEIDON_POINTER_COUNTER_H
#define POSEIDON_POINTER_COUNTER_H

#include "poseidon/pointer.h"
#include "poseidon/relaxed_atomic.h"

namespace poseidon {
 class PointerCounter {
  private:
   RelaxedAtomic<word> count_;
   RelaxedAtomic<word> bytes_;
  public:
   PointerCounter() = default;
   PointerCounter(const PointerCounter& rhs) = default;
   ~PointerCounter() = default;

   word count() const {
     return (word)count_;
   }

   word bytes() const {
     return (word)bytes_;
   }

   void clear() {
     count_ = bytes_ = 0;
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

   friend std::ostream& operator<<(std::ostream& stream, const PointerCounter& value) {
     return stream << value.count() << " (" << Bytes(value.bytes()) << ")";
   }
 };
}

#endif // POSEIDON_POINTER_COUNTER_H