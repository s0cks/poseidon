#include <gtest/gtest.h>

#include "poseidon/type.h"

namespace poseidon {
 using namespace ::testing;

 class LongTest : public Test {
  protected:
   LongTest() = default;
  public:
   ~LongTest() override = default;
 };

 TEST_F(LongTest, TestNew_WillPass) {
   static const constexpr RawLong kAValue = 34;
   auto a = Long::New(kAValue);
   DLOG(INFO) << "a: " << (*a);
   ASSERT_EQ(a->Get(), kAValue);

   static const constexpr RawLong kBValue = 59;
   auto b = Long::New(kBValue);
   DLOG(INFO) << "b: " << (*b);
   ASSERT_EQ(b->Get(), kBValue);
 }
}