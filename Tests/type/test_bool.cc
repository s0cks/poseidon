#include <gtest/gtest.h>

#include "poseidon/type.h"

namespace poseidon {
 using namespace ::testing;

 class BoolTest : public Test {
  protected:
   BoolTest() = default;
  public:
   ~BoolTest() override = default;
 };

 TEST_F(BoolTest, TestTrue_WillPass) {
   auto value = Bool::True();
   DLOG(INFO) << "value: " << (*value);
   ASSERT_TRUE(value->Get());
 }

 TEST_F(BoolTest, TestFalse_WillPass) {
   auto value = Bool::False();
   DLOG(INFO) << "value: " << (*value);
   ASSERT_FALSE(value->Get());
 }
}