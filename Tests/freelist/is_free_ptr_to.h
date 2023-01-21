#ifndef POSEIDON_IS_FREE_POINTER_TO_H
#define POSEIDON_IS_FREE_POINTER_TO_H

#include <gtest/gtest.h>
#include "poseidon/freelist/free_ptr.h"

namespace poseidon {
 class IsFreePointerToMatcher {
  protected:
   Region region_;
  public:
   using is_gtest_matcher = void;

   explicit IsFreePointerToMatcher(const Region& region):
       region_(region) { }
   explicit IsFreePointerToMatcher(const uword start, const RegionSize size):
     region_(start, size) { }
   ~IsFreePointerToMatcher() = default;

   bool MatchAndExplain(FreePointer* actual, std::ostream*) const {
     return region_ == ((const Region&)*actual);
   }

   void DescribeTo(std::ostream* stream) const {
     (*stream) << "free pointer points to " << region_;
   }

   void DescribeNegationTo(std::ostream* stream) const {
     (*stream) << "free pointer does not point to " << region_;
   }
 };

 static inline testing::Matcher<FreePointer*>
 IsFreePointerTo(const Region& region) {
   return IsFreePointerToMatcher(region);
 }

 static inline testing::Matcher<FreePointer*>
 IsFreePointerTo(const uword start, const RegionSize size) {
   return IsFreePointerTo(Region(start, size));
 }
}

#endif //POSEIDON_IS_FREE_POINTER_TO_H