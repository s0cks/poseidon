#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <glog/logging.h>

#include "poseidon/heap/page_tag.h"

namespace poseidon {
 using namespace ::testing;

 class PageTagTest : public Test {
  protected:
   PageTagTest() = default;

   static inline void
   SetMarkedBit(PageTag& tag, const bool value = true) {
     return tag.SetMarkedBit(value);
   }

   static inline void
   SetNewBit(PageTag& tag, const bool value = true) {
     return tag.SetNewBit(value);
   }

   static inline void
   SetOldBit(PageTag& tag, const bool value = true) {
     return tag.SetOldBit(value);
   }

   static inline void
   SetIndex(PageTag& tag, const PageIndex value) {
     return tag.SetIndex(value);
   }
  public:
   ~PageTagTest() override = default;
 };

 TEST_F(PageTagTest, TestConstructor) {
   PageTag tag;
   ASSERT_LE(PageTag::kTotalBits, kWordSizeInBits);
   ASSERT_EQ(sizeof(RawPageTag), kWordSize);
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
   static const constexpr PageIndex kPageIndex = 1;
   static const constexpr PageTag kTag1 = PageTag::NewMarked(kPageIndex);
   LOG(INFO) << "kTag1: " << kTag1;
   static const constexpr PageTag kTag2 = PageTag::OldMarked(kPageIndex);
   ASSERT_NE(kTag1, kTag2);
 }

 TEST_F(PageTagTest, TestMarkedBit) {
   PageTag tag;
   ASSERT_FALSE(tag.IsMarked());
   ASSERT_NO_FATAL_FAILURE(SetMarkedBit(tag, true));
   ASSERT_TRUE(tag.IsMarked());
   ASSERT_NO_FATAL_FAILURE(SetMarkedBit(tag, false));
   ASSERT_FALSE(tag.IsMarked());
 }

 TEST_F(PageTagTest, TestNewBit) {
   PageTag tag;
   ASSERT_FALSE(tag.IsNew());
   ASSERT_NO_FATAL_FAILURE(SetNewBit(tag, true));
   ASSERT_TRUE(tag.IsNew());
   ASSERT_NO_FATAL_FAILURE(SetNewBit(tag, false));
   ASSERT_FALSE(tag.IsNew());
 }

 TEST_F(PageTagTest, TestOldBit) {
   PageTag tag;
   ASSERT_FALSE(tag.IsOld());
   ASSERT_NO_FATAL_FAILURE(SetOldBit(tag, true));
   ASSERT_TRUE(tag.IsOld());
   ASSERT_NO_FATAL_FAILURE(SetOldBit(tag, false));
   ASSERT_FALSE(tag.IsOld());
 }

 TEST_F(PageTagTest, TestIndexTag) {
   static const constexpr int64_t kDefaultIndexValue = 32;
   PageTag tag;
   ASSERT_EQ(tag.GetIndex(), 0);
   ASSERT_NO_FATAL_FAILURE(SetIndex(tag, kDefaultIndexValue));
   ASSERT_EQ(tag.GetIndex(), kDefaultIndexValue);
   ASSERT_NO_FATAL_FAILURE(SetIndex(tag, 0));
   ASSERT_EQ(tag.GetIndex(), 0);
 }
}
