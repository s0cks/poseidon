#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "poseidon/heap/page_tag.h"

namespace poseidon {
 using namespace ::testing;

 class PageTagTest : public Test {
  protected:
   PageTagTest() = default;
  public:
   ~PageTagTest() override = default;
 };

 TEST_F(PageTagTest, TestConstructor) {
   PageTag tag;
   ASSERT_EQ(tag.raw(), kInvalidPageTag);
   ASSERT_FALSE(tag.IsMarked());
   ASSERT_FALSE(tag.IsNew());
   ASSERT_FALSE(tag.IsOld());
   ASSERT_EQ(tag.GetIndex(), 0);
 }

 TEST_F(PageTagTest, TestEquals) {
   PageTag a(PageTag::NewBit::Encode(true) | PageTag::IndexTag::Encode(1));
   PageTag b(PageTag::NewBit::Encode(true) | PageTag::IndexTag::Encode(1));
   ASSERT_EQ(a, b);
 }

 TEST_F(PageTagTest, TestNotEquals) {
   PageTag a(PageTag::NewBit::Encode(true) | PageTag::IndexTag::Encode(1));
   PageTag b(PageTag::OldBit::Encode(true) | PageTag::IndexTag::Encode(1));
   ASSERT_NE(a, b);
 }

 TEST_F(PageTagTest, TestIndexTag) {
   static const constexpr int64_t kDefaultIndexValue = 32;
   PageTag tag;
   ASSERT_EQ(tag.GetIndex(), 0);
   ASSERT_NO_FATAL_FAILURE(tag.SetIndex(kDefaultIndexValue));
   ASSERT_EQ(tag.GetIndex(), kDefaultIndexValue);
 }

 TEST_F(PageTagTest, TestMarkedBit) {
   PageTag tag;
   ASSERT_FALSE(tag.IsMarked());
   ASSERT_NO_FATAL_FAILURE(tag.SetMarkedBit());
   ASSERT_TRUE(tag.IsMarked());
   ASSERT_NO_FATAL_FAILURE(tag.ClearMarkedBit());
   ASSERT_FALSE(tag.IsMarked());
 }

 TEST_F(PageTagTest, TestNewBit) {
   PageTag tag;
   ASSERT_FALSE(tag.IsNew());
   ASSERT_NO_FATAL_FAILURE(tag.SetNewBit());
   ASSERT_TRUE(tag.IsNew());
   ASSERT_NO_FATAL_FAILURE(tag.ClearNewBit());
   ASSERT_FALSE(tag.IsNew());
 }

 TEST_F(PageTagTest, TestOldBit) {
   PageTag tag;
   ASSERT_FALSE(tag.IsOld());
   ASSERT_NO_FATAL_FAILURE(tag.SetOldBit());
   ASSERT_TRUE(tag.IsOld());
   ASSERT_NO_FATAL_FAILURE(tag.ClearOldBit());
   ASSERT_FALSE(tag.IsOld());
 }
}
