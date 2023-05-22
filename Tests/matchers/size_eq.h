#ifndef PSDN_SIZE_EQ_H
#define PSDN_SIZE_EQ_H

#include <gtest/gtest.h>
#include "poseidon/common.h"

namespace poseidon {
 class SizeEqMatcher {
  protected:
   const SizeType size_;
  public:
   using is_gtest_matcher = void;

   explicit SizeEqMatcher(const SizeType size):
     size_(size) { }
   virtual ~SizeEqMatcher() = default;

   SizeType size() const {
     return size_;
   }

   bool MatchAndExplain(const SizeType actual, std::ostream*) const {
     return actual == size();
   }

   void DescribeTo(std::ostream* stream) const {
     (*stream) << "size equals " << Bytes(size());
   }

   void DescribeNegationTo(std::ostream* stream) const {
     (*stream) << "size does not equal " << Bytes(size());
   }
 };

 static inline ::testing::Matcher<SizeType>
 SizeEq(const SizeType expected) {
   return SizeEqMatcher(expected);
 }
}

#endif //PSDN_SIZE_EQ_H