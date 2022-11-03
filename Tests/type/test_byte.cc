#include <gtest/gtest.h>

#include "poseidon/type/byte.h"

namespace poseidon {
 using namespace ::testing;

 class ByteTest : public Test {
  protected:
   ByteTest() = default;
  public:
   ~ByteTest() override = default;
 };

 TEST_F(ByteTest, TestNew_WillPass) {
   static const constexpr RawByte kAValue = 34;
   auto a = Byte::New(kAValue);
   DLOG(INFO) << "a: " << (*a);
   ASSERT_EQ(a->Get(), kAValue);

   static const constexpr RawByte kBValue = 59;
   auto b = Byte::New(kBValue);
   DLOG(INFO) << "b: " << (*b);
   ASSERT_EQ(b->Get(), kBValue);
 }
}