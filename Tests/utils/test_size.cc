#include <gtest/gtest.h>

#include "poseidon/utils/size.h"

namespace poseidon {
 using namespace ::testing;

 class SizeTest : public Test {
  protected:
   SizeTest() = default;
  public:
   ~SizeTest() override = default;
 };

#define DEFINE_SIZE_PARSE_TEST(Name, Expected, Text) \
  TEST_F(SizeTest, TestParse_##Name) {               \
    static constexpr const Size kExpectedSize = (Expected); \
    Size result;                                     \
    ASSERT_TRUE(SizeParser::ParseSize((Text), &result));    \
    ASSERT_EQ(kExpectedSize, result);                \
  }

 DEFINE_SIZE_PARSE_TEST(TestBytes1, Bytes(16), "16b");
 DEFINE_SIZE_PARSE_TEST(TestBytes2, Bytes(16), "16B");

 DEFINE_SIZE_PARSE_TEST(TestKilobytes1, Kilobytes(16), "16kb");
 DEFINE_SIZE_PARSE_TEST(TestKilobytes2, Kilobytes(16), "16Kb");
 DEFINE_SIZE_PARSE_TEST(TestKilobytes3, Kilobytes(16), "16kB");
 DEFINE_SIZE_PARSE_TEST(TestKilobytes4, Kilobytes(16), "16KB");

 DEFINE_SIZE_PARSE_TEST(TestMegabytes1, Megabytes(16), "16mb");
 DEFINE_SIZE_PARSE_TEST(TestMegabytes2, Megabytes(16), "16Mb");
 DEFINE_SIZE_PARSE_TEST(TestMegabytes3, Megabytes(16), "16mB");
 DEFINE_SIZE_PARSE_TEST(TestMegabytes4, Megabytes(16), "16MB");

 DEFINE_SIZE_PARSE_TEST(TestGigabytes1, Gigabytes(16), "16gb");
 DEFINE_SIZE_PARSE_TEST(TestGigabytes2, Gigabytes(16), "16Gb");
 DEFINE_SIZE_PARSE_TEST(TestGigabytes3, Gigabytes(16), "16gB");
 DEFINE_SIZE_PARSE_TEST(TestGigabytes4, Gigabytes(16), "16GB");
}