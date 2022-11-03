#include <gtest/gtest.h>

#include "poseidon/type/int.h"

namespace poseidon {
 using namespace ::testing;

 class IntTest : public Test {
  protected:
   IntTest() = default;
  public:
   ~IntTest() override = default;
 };

 TEST_F(IntTest, TestNew_WillPass) {
   static const constexpr RawInt kAValue = 34;
   auto a = Int::New(kAValue);
   DLOG(INFO) << "a: " << (*a);
   ASSERT_EQ(a->Get(), kAValue);

   static const constexpr RawInt kBValue = 59;
   auto b = Int::New(kBValue);
   DLOG(INFO) << "b: " << (*b);
   ASSERT_EQ(b->Get(), kBValue);
 }
}