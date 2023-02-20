#include "page/test_new_page.h"

namespace poseidon {
 TEST_F(NewPageTest, TestGetIndex) {
   ASSERT_EQ(kTestPageIndex, page().GetIndex());
 }

 TEST_F(NewPageTest, TestGetSize) {
   ASSERT_EQ((RegionSize) kTestPageSize, page().GetSize());
 }

 TEST_F(NewPageTest, TestGetStatingAddress) {
   ASSERT_EQ(region().GetStartingAddress(), page().GetStartingAddress());
 }

 TEST_F(NewPageTest, TestGetEndingAddress) {
   ASSERT_EQ(region().GetEndingAddress(), page().GetEndingAddress());
 }
}