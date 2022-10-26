#include "heap/test_page_tag.h"

namespace poseidon {
 TEST_F(PageTagTest, TestMarkedBit) {
   PageTag tag;
   ASSERT_EQ(tag.raw(), kInvalidPageTag);
   ASSERT_FALSE(tag.IsMarked());
   ASSERT_EQ(tag.GetIndex(), 0);

   ASSERT_NO_FATAL_FAILURE(tag.SetMarkedBit());

   ASSERT_NE(tag.raw(), kInvalidPageTag);
   ASSERT_TRUE(tag.IsMarked());
   ASSERT_EQ(tag.GetIndex(), 0);
 }

 TEST_F(PageTagTest, TestIndexTag) {
   static constexpr const uint32_t kIndex = 4;

   PageTag tag;
   ASSERT_EQ(tag.raw(), kInvalidPageTag);
   ASSERT_FALSE(tag.IsMarked());
   ASSERT_EQ(tag.GetIndex(), 0);

   ASSERT_NO_FATAL_FAILURE(tag.SetIndex(kIndex));

   ASSERT_NE(tag.raw(), kInvalidPageTag);
   ASSERT_FALSE(tag.IsMarked());
   ASSERT_EQ(tag.GetIndex(), kIndex);
 }
}
