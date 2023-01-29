#include <gtest/gtest.h>
#include "poseidon/type.h"

namespace poseidon {
 using namespace ::testing;

 class ShortTest : public Test {
  protected:
   ShortTest() = default;
  public:
   ~ShortTest() override = default;
 };

 TEST_F(ShortTest, TestNew_WillPass) {
   static const constexpr RawShort kAValue = 459;
   auto a = Short::New(kAValue);
   DLOG(INFO) << "a: " << (*a);
   ASSERT_EQ(a->Get(), kAValue);

   static const constexpr RawShort kBValue = 192;
   auto b = Short::New(kBValue);
   DLOG(INFO) << "b: " << (*b);
   ASSERT_EQ(b->Get(), kBValue);
 }
}