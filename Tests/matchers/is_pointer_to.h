#ifndef POSEIDON_IS_POINTER_TO_MATCHER_H
#define POSEIDON_IS_POINTER_TO_MATCHER_H

#include <gtest/gtest.h>

#include "poseidon/pointer.h"
#include "poseidon/local/local.h"

namespace poseidon {
 class IsPointerToMatcher {
  protected:
   const uword start_;
   const PointerTag tag_;
  public:
   using is_gtest_matcher = void;

   IsPointerToMatcher(const uword start,
                      const PointerTag& tag):
     start_(start),
     tag_(tag) {
   }

   uword starting_address() const {
     return start_;
   }

   PointerTag tag() const {
     return tag_;
   }

   bool MatchAndExplain(Pointer* ptr, std::ostream*) const {
     return ptr->GetStartingAddress() == starting_address() &&
            ptr->tag() == tag();
   }

   void DescribeTo(std::ostream* stream) const {
     (*stream) << "pointer points to " << ((void*) starting_address()) << " (" << tag() << ")";
   }

   void DescribeNegationTo(std::ostream* stream) const {
     (*stream) << "pointer does not point to " << ((void*) starting_address()) << " (" << tag() << ")";
   }
 };

 static inline ::testing::Matcher<Pointer*>
 IsPointerTo(const uword start, const PointerTag tag) {
   return IsPointerToMatcher(start, tag);
 }

 static inline ::testing::Matcher<Pointer*>
 IsPointerTo(Pointer* ptr) {
   return IsPointerTo(ptr->GetStartingAddress(), ptr->tag());
 }

 template<class T>
 static inline ::testing::Matcher<Pointer*>
 IsPointerTo(const Local<T>& val) {
   return IsPointerTo(val.raw_ptr());
 }

 template<class T>
 static inline ::testing::Matcher<Pointer*>
 IsPointerTo(T* val) {
   return IsPointerTo((Pointer*) val->raw_ptr());
 }
}

#endif // POSEIDON_IS_POINTER_TO_MATCHER_H