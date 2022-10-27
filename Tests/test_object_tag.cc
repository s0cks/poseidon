#include <gtest/gtest.h>
#include "poseidon/object_tag.h"

namespace poseidon {
 using namespace ::testing;

 class ObjectTagTest : public Test {
  protected:
   ObjectTagTest() = default;
  public:
   ~ObjectTagTest() override = default;
 };

 TEST_F(ObjectTagTest, TestConstructor) {
   static const constexpr ObjectSize kObjectSize = 0;
   static const constexpr ObjectTag kTag = ObjectTag::Empty();
   ASSERT_FALSE(kTag.IsNew());
   ASSERT_FALSE(kTag.IsOld());
   ASSERT_FALSE(kTag.IsMarked());
   ASSERT_FALSE(kTag.IsRemembered());
   ASSERT_EQ(kTag.GetSize(), kObjectSize);
 }

 TEST_F(ObjectTagTest, TestEquals) {
   static const constexpr ObjectSize kObjectSize = kWordSize;
   static const constexpr ObjectTag kTag1 = ObjectTag::New(kObjectSize);
   static const constexpr ObjectTag kTag2 = ObjectTag::New(kObjectSize);
   ASSERT_EQ(kTag1, kTag2);
 }

 TEST_F(ObjectTagTest, TestNotEquals) {
   static const constexpr ObjectSize kObjectSize = kWordSize;
   static const constexpr ObjectTag kTag1 = ObjectTag::New(kObjectSize);
   static const constexpr ObjectTag kTag2 = ObjectTag::Old(kObjectSize);
   ASSERT_NE(kTag1, kTag2);
 }

 TEST_F(ObjectTagTest, TestMarkedBit) {
   ObjectTag kTag = ObjectTag::Empty();
   ASSERT_FALSE(kTag.IsMarked());
   ASSERT_NO_FATAL_FAILURE(kTag.SetMarked());
   ASSERT_TRUE(kTag.IsMarked());
   ASSERT_NO_FATAL_FAILURE(kTag.ClearMarked());
   ASSERT_FALSE(kTag.IsMarked());
 }

 TEST_F(ObjectTagTest, TestRememberedBit) {
   ObjectTag kTag = ObjectTag::Empty();
   ASSERT_FALSE(kTag.IsRemembered());
   ASSERT_NO_FATAL_FAILURE(kTag.SetRemembered());
   ASSERT_TRUE(kTag.IsRemembered());
   ASSERT_NO_FATAL_FAILURE(kTag.ClearRemembered());
   ASSERT_FALSE(kTag.IsRemembered());
 }

 TEST_F(ObjectTagTest, TestNewBit) {
   ObjectTag kTag = ObjectTag::Empty();
   ASSERT_FALSE(kTag.IsNew());
   ASSERT_NO_FATAL_FAILURE(kTag.SetNew());
   ASSERT_TRUE(kTag.IsNew());
   ASSERT_NO_FATAL_FAILURE(kTag.ClearNew());
   ASSERT_FALSE(kTag.IsNew());
 }

 TEST_F(ObjectTagTest, TestOldBit) {
   ObjectTag kTag = ObjectTag::Empty();
   ASSERT_FALSE(kTag.IsOld());
   ASSERT_NO_FATAL_FAILURE(kTag.SetOld());
   ASSERT_TRUE(kTag.IsOld());
   ASSERT_NO_FATAL_FAILURE(kTag.ClearOld());
   ASSERT_FALSE(kTag.IsOld());
 }

 TEST_F(ObjectTagTest, TestSizeTag) {
   ObjectTag kTag = ObjectTag::Empty();
   ASSERT_EQ(kTag.GetSize(), 0);
   ASSERT_NO_FATAL_FAILURE(kTag.SetSize(kWordSize));
   ASSERT_EQ(kTag.GetSize(), kWordSize);
   ASSERT_NO_FATAL_FAILURE(kTag.ClearSize());
   ASSERT_EQ(kTag.GetSize(), 0);
 }
}