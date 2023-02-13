#include <gtest/gtest.h>
#include <glog/logging.h>

#include "poseidon/pointer_tag.h"

namespace poseidon {
 using namespace ::testing;

 class PointerTagTest : public Test {
  protected:
   PointerTagTest() = default;
  public:
   ~PointerTagTest() override = default;
 };

 TEST_F(PointerTagTest, TestTagSize) {
   static constexpr const auto kRawObjectTagSize = sizeof(RawPointerTag);
   static constexpr const auto kRawObjectTagSizeInBits = kRawObjectTagSize * kBitsPerByte;
   static constexpr const auto kUsedBits = PointerTag::kTotalBits;
   static constexpr const auto kRemainingBits = kRawObjectTagSizeInBits - kUsedBits;
   DLOG(INFO) << "sizeof(RawPointerTag): " << Bytes(kRawObjectTagSize) << " (" << kUsedBits << "/" << kRawObjectTagSizeInBits << " bits used, " << kRemainingBits << " remaining)";
   ASSERT_EQ(sizeof(RawPointerTag), sizeof(uword));
   ASSERT_EQ(sizeof(PointerTag), sizeof(RawPointerTag));
   ASSERT_LE(PointerTag::kTotalBits, kBitsPerWord);
 }

 TEST_F(PointerTagTest, TestConstructor) {
   static const constexpr ObjectSize kObjectSize = 0;
   static const constexpr PointerTag kTag = PointerTag::Empty();
   ASSERT_FALSE(kTag.IsNew());
   ASSERT_FALSE(kTag.IsOld());
   ASSERT_FALSE(kTag.IsMarked());
   ASSERT_FALSE(kTag.IsRemembered());
   ASSERT_FALSE(kTag.IsFree());
   ASSERT_EQ(kTag.GetSize(), kObjectSize);
 }

 TEST_F(PointerTagTest, TestEquals) {
   static const constexpr ObjectSize kObjectSize = kWordSize;
   static const constexpr PointerTag kTag1 = PointerTag::New(kObjectSize);
   static const constexpr PointerTag kTag2 = PointerTag::New(kObjectSize);
   ASSERT_EQ(kTag1, kTag2);
 }

 TEST_F(PointerTagTest, TestNotEquals) {
   static const constexpr ObjectSize kObjectSize = kWordSize;
   static const constexpr PointerTag kTag1 = PointerTag::New(kObjectSize);
   static const constexpr PointerTag kTag2 = PointerTag::Old(kObjectSize);
   ASSERT_NE(kTag1, kTag2);
 }

 TEST_F(PointerTagTest, TestMarkedBit) {
   PointerTag kTag = PointerTag::Empty();
   ASSERT_FALSE(kTag.IsMarked());
   ASSERT_NO_FATAL_FAILURE(kTag.SetMarked());
   ASSERT_TRUE(kTag.IsMarked());
   ASSERT_NO_FATAL_FAILURE(kTag.ClearMarked());
   ASSERT_FALSE(kTag.IsMarked());
 }

 TEST_F(PointerTagTest, TestRememberedBit) {
   PointerTag kTag = PointerTag::Empty();
   ASSERT_FALSE(kTag.IsRemembered());
   ASSERT_NO_FATAL_FAILURE(kTag.SetRemembered());
   ASSERT_TRUE(kTag.IsRemembered());
   ASSERT_NO_FATAL_FAILURE(kTag.ClearRemembered());
   ASSERT_FALSE(kTag.IsRemembered());
 }

 TEST_F(PointerTagTest, TestNewBit) {
   PointerTag kTag = PointerTag::Empty();
   ASSERT_FALSE(kTag.IsNew());
   ASSERT_NO_FATAL_FAILURE(kTag.SetNew());
   ASSERT_TRUE(kTag.IsNew());
   ASSERT_NO_FATAL_FAILURE(kTag.ClearNew());
   ASSERT_FALSE(kTag.IsNew());
 }

 TEST_F(PointerTagTest, TestOldBit) {
   PointerTag kTag = PointerTag::Empty();
   ASSERT_FALSE(kTag.IsOld());
   ASSERT_NO_FATAL_FAILURE(kTag.SetOld());
   ASSERT_TRUE(kTag.IsOld());
   ASSERT_NO_FATAL_FAILURE(kTag.ClearOld());
   ASSERT_FALSE(kTag.IsOld());
 }

 TEST_F(PointerTagTest, TestFreeBit) {
   PointerTag kTag = PointerTag::Empty();
   ASSERT_FALSE(kTag.IsFree());
   ASSERT_NO_FATAL_FAILURE(kTag.SetFree(true));
   ASSERT_TRUE(kTag.IsFree());
   ASSERT_NO_FATAL_FAILURE(kTag.SetFree(false));
   ASSERT_FALSE(kTag.IsFree());
 }

 TEST_F(PointerTagTest, TestSizeTag) {
   PointerTag kTag = PointerTag::Empty();
   ASSERT_EQ(kTag.GetSize(), 0);
   ASSERT_NO_FATAL_FAILURE(kTag.SetSize(kWordSize));
   ASSERT_EQ(kTag.GetSize(), kWordSize);
   ASSERT_NO_FATAL_FAILURE(kTag.ClearSize());
   ASSERT_EQ(kTag.GetSize(), 0);
 }
}