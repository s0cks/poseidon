#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <glog/logging.h>

#include "poseidon/bitset.h"
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

 TEST_F(PageTagTest, TestConstructor_EmptyTag) {
   PageTag tag = PageTag::Empty();
   ASSERT_EQ(tag.raw(), kInvalidPageTag);
   ASSERT_FALSE(tag.IsMarked());
   ASSERT_FALSE(tag.IsNew());
   ASSERT_FALSE(tag.IsOld());
   ASSERT_EQ(tag.GetIndex(), 0);
   ASSERT_EQ(tag.GetSize(), 0);
 }

 TEST_F(PageTagTest, TestConstructor_NewTag) {
   static const constexpr PageIndex kTag1Index = 0;
   static const constexpr ObjectSize kTag1Size = 0;
   static const constexpr PageTag kTag1 = PageTag::New(kTag1Index, kTag1Size);
   ASSERT_NE(kTag1.raw(), kInvalidPageTag);
   ASSERT_FALSE(kTag1.IsMarked());
   ASSERT_TRUE(kTag1.IsNew());
   ASSERT_FALSE(kTag1.IsOld());
   ASSERT_EQ(kTag1.GetIndex(), kTag1Index);
   ASSERT_EQ(kTag1.GetSize(), kTag1Size);

   static const constexpr PageIndex kTag2Index = 1;
   static const constexpr ObjectSize kTag2Size = 0;
   static const constexpr PageTag kTag2 = PageTag::New(kTag2Index, kTag2Size);
   ASSERT_NE(kTag2.raw(), kInvalidPageTag);
   ASSERT_FALSE(kTag2.IsMarked());
   ASSERT_TRUE(kTag2.IsNew());
   ASSERT_FALSE(kTag2.IsOld());
   ASSERT_EQ(kTag2.GetIndex(), kTag2Index);
   ASSERT_EQ(kTag2.GetSize(), kTag2Size);

   static const constexpr PageIndex kTag3Index = 1;
   static const constexpr ObjectSize kTag3Size = 8 * kMB;
   static const constexpr PageTag kTag3 = PageTag::New(kTag3Index, kTag3Size);
   ASSERT_NE(kTag3.raw(), kInvalidPageTag);
   ASSERT_FALSE(kTag3.IsMarked());
   ASSERT_TRUE(kTag3.IsNew());
   ASSERT_FALSE(kTag3.IsOld());
   ASSERT_EQ(kTag3.GetIndex(), kTag3Index);
   ASSERT_EQ(kTag3.GetSize(), kTag3Size);
 }

 TEST_F(PageTagTest, TestConstructor_NewMarkedTag) {
   static const constexpr PageIndex kTag1Index = 0;
   static const constexpr ObjectSize kTag1Size = 0;
   static const constexpr PageTag kTag1 = PageTag::NewMarked(kTag1Index, kTag1Size);
   ASSERT_NE(kTag1.raw(), kInvalidPageTag);
   ASSERT_TRUE(kTag1.IsMarked());
   ASSERT_TRUE(kTag1.IsNew());
   ASSERT_FALSE(kTag1.IsOld());
   ASSERT_EQ(kTag1.GetIndex(), kTag1Index);
   ASSERT_EQ(kTag1.GetSize(), kTag1Size);

   static const constexpr PageIndex kTag2Index = 1;
   static const constexpr ObjectSize kTag2Size = 0;
   static const constexpr PageTag kTag2 = PageTag::NewMarked(kTag2Index, kTag2Size);
   ASSERT_NE(kTag2.raw(), kInvalidPageTag);
   ASSERT_TRUE(kTag2.IsMarked());
   ASSERT_TRUE(kTag2.IsNew());
   ASSERT_FALSE(kTag2.IsOld());
   ASSERT_EQ(kTag2.GetIndex(), kTag2Index);
   ASSERT_EQ(kTag2.GetSize(), kTag2Size);

   static const constexpr PageIndex kTag3Index = 1;
   static const constexpr ObjectSize kTag3Size = 8 * kMB;
   static const constexpr PageTag kTag3 = PageTag::NewMarked(kTag3Index, kTag3Size);
   ASSERT_NE(kTag3.raw(), kInvalidPageTag);
   ASSERT_TRUE(kTag3.IsMarked());
   ASSERT_TRUE(kTag3.IsNew());
   ASSERT_FALSE(kTag3.IsOld());
   ASSERT_EQ(kTag3.GetIndex(), kTag3Index);
   ASSERT_EQ(kTag3.GetSize(), kTag3Size);
 }

 TEST_F(PageTagTest, TestConstructor_OldTag) {
   static const constexpr PageIndex kTag1Index = 0;
   static const constexpr ObjectSize kTag1Size = 0;
   static const constexpr PageTag kTag1 = PageTag::Old(kTag1Index, kTag1Size);
   ASSERT_NE(kTag1.raw(), kInvalidPageTag);
   ASSERT_FALSE(kTag1.IsMarked());
   ASSERT_FALSE(kTag1.IsNew());
   ASSERT_TRUE(kTag1.IsOld());
   ASSERT_EQ(kTag1.GetIndex(), kTag1Index);
   ASSERT_EQ(kTag1.GetSize(), kTag1Size);

   static const constexpr PageIndex kTag2Index = 1;
   static const constexpr ObjectSize kTag2Size = 0;
   static const constexpr PageTag kTag2 = PageTag::Old(kTag2Index, kTag2Size);
   ASSERT_NE(kTag2.raw(), kInvalidPageTag);
   ASSERT_FALSE(kTag2.IsMarked());
   ASSERT_FALSE(kTag1.IsNew());
   ASSERT_TRUE(kTag1.IsOld());
   ASSERT_EQ(kTag2.GetIndex(), kTag2Index);
   ASSERT_EQ(kTag2.GetSize(), kTag2Size);

   static const constexpr PageIndex kTag3Index = 1;
   static const constexpr ObjectSize kTag3Size = 8 * kMB;
   static const constexpr PageTag kTag3 = PageTag::Old(kTag3Index, kTag3Size);
   ASSERT_NE(kTag3.raw(), kInvalidPageTag);
   ASSERT_FALSE(kTag3.IsMarked());
   ASSERT_FALSE(kTag1.IsNew());
   ASSERT_TRUE(kTag1.IsOld());
   ASSERT_EQ(kTag3.GetIndex(), kTag3Index);
   ASSERT_EQ(kTag3.GetSize(), kTag3Size);
 }

 TEST_F(PageTagTest, TestConstructor_OldMarkedTag) {
   static const constexpr PageIndex kTag1Index = 0;
   static const constexpr ObjectSize kTag1Size = 0;
   static const constexpr PageTag kTag1 = PageTag::OldMarked(kTag1Index, kTag1Size);
   ASSERT_NE(kTag1.raw(), kInvalidPageTag);
   ASSERT_TRUE(kTag1.IsMarked());
   ASSERT_FALSE(kTag1.IsNew());
   ASSERT_TRUE(kTag1.IsOld());
   ASSERT_EQ(kTag1.GetIndex(), kTag1Index);
   ASSERT_EQ(kTag1.GetSize(), kTag1Size);

   static const constexpr PageIndex kTag2Index = 1;
   static const constexpr ObjectSize kTag2Size = 0;
   static const constexpr PageTag kTag2 = PageTag::OldMarked(kTag2Index, kTag2Size);
   ASSERT_NE(kTag2.raw(), kInvalidPageTag);
   ASSERT_TRUE(kTag2.IsMarked());
   ASSERT_FALSE(kTag1.IsNew());
   ASSERT_TRUE(kTag1.IsOld());
   ASSERT_EQ(kTag2.GetIndex(), kTag2Index);
   ASSERT_EQ(kTag2.GetSize(), kTag2Size);

   static const constexpr PageIndex kTag3Index = 1;
   static const constexpr ObjectSize kTag3Size = 8 * kMB;
   static const constexpr PageTag kTag3 = PageTag::OldMarked(kTag3Index, kTag3Size);
   ASSERT_NE(kTag3.raw(), kInvalidPageTag);
   ASSERT_TRUE(kTag3.IsMarked());
   ASSERT_FALSE(kTag1.IsNew());
   ASSERT_TRUE(kTag1.IsOld());
   ASSERT_EQ(kTag3.GetIndex(), kTag3Index);
   ASSERT_EQ(kTag3.GetSize(), kTag3Size);
 }

 TEST_F(PageTagTest, TestEquals) {
   static const constexpr PageIndex kPageIndex = 1;
   static const constexpr PageTag kTag1 = PageTag::NewMarked(kPageIndex);
   static const constexpr PageTag kTag2 = PageTag::NewMarked(kPageIndex);
   ASSERT_EQ(kTag1, kTag2);
 }

 TEST_F(PageTagTest, TestNotEquals) {
   static const constexpr PageIndex kPageIndex = 1;
   static const constexpr PageTag kTag1 = PageTag::NewMarked(kPageIndex);
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
