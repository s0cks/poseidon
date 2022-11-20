#ifndef POSEIDON_IS_POINTER_TO_MATCHER_H
#define POSEIDON_IS_POINTER_TO_MATCHER_H

#include <gtest/gtest.h>

#include "poseidon/raw_object.h"
#include "poseidon/local/local.h"

namespace poseidon {
 class IsPointerToMatcher {
  protected:
   const uword start_;
   const ObjectSize size_;
  public:
   using is_gtest_matcher = void;

   IsPointerToMatcher(const uword start,
                      const ObjectSize size):
       start_(start),
       size_(size) {
   }

   uword starting_address() const {
     return start_;
   }

   ObjectSize size() const {
     return size_;
   }

   bool MatchAndExplain(RawObject* ptr, std::ostream*) const {
     return ptr->GetStartingAddress() == starting_address() &&
            ptr->GetSize() == size();
   }

   void DescribeTo(std::ostream* stream) const {
     (*stream) << "pointer points to " << ((void*) starting_address()) << " (" << Bytes(size()) << ")";
   }

   void DescribeNegationTo(std::ostream* stream) const {
     (*stream) << "pointer does not point to " << ((void*) starting_address()) << " (" << Bytes(size()) << ")";
   }
 };

 static inline ::testing::Matcher<RawObject*>
 IsPointerTo(const uword start, const int64_t size) {
   return IsPointerToMatcher(start, size);
 }

 static inline ::testing::Matcher<RawObject*>
 IsPointerTo(RawObject* ptr) {
   return IsPointerTo(ptr->GetStartingAddress(), ptr->GetSize());
 }

 template<class T>
 static inline ::testing::Matcher<RawObject*>
 IsPointerTo(const Local<T>& val) {
   return IsPointerTo(val.raw_ptr());
 }

 template<class T>
 static inline ::testing::Matcher<RawObject*>
 IsPointerTo(T* val) {
   return IsPointerTo((RawObject*) val->raw_ptr());
 }
}

#endif // POSEIDON_IS_POINTER_TO_MATCHER_H