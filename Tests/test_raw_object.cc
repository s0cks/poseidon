#include <gtest/gtest.h>

#include "helpers.h"
#include "poseidon/raw_object.h"

namespace poseidon{
 using namespace ::testing;

 class ObjectTagTest : public Test{
  public:
   ObjectTagTest() = default;
   ~ObjectTagTest() override = default;
 };

 TEST_F(ObjectTagTest, TestSize){
   ASSERT_EQ(sizeof(RawObjectTag), sizeof(uword)) << "RawObjectTag should be the size of a uword.";
   ASSERT_EQ(sizeof(ObjectTag), sizeof(RawObjectTag)) << "ObjectTag should be the size of a uword.";
 }

 TEST_F(ObjectTagTest, TestNewBit){
   ObjectTag tag;
   ASSERT_FALSE(tag.IsNew()) << "NewBit should be false by default.";
   tag.SetNew();
   ASSERT_TRUE(tag.IsNew()) << "NewBit should be true.";
   tag.ClearNew();
   ASSERT_FALSE(tag.IsNew()) << "NewBit should be false.";
 }

 TEST_F(ObjectTagTest, TestOldBit){
   ObjectTag tag;
   ASSERT_FALSE(tag.IsOld());
   tag.SetOld();
   ASSERT_TRUE(tag.IsOld());
   tag.ClearOld();
   ASSERT_FALSE(tag.IsOld());
 }

 TEST_F(ObjectTagTest, TestMarkedBit){
   ObjectTag tag;
   ASSERT_FALSE(tag.IsMarked());
   tag.SetMarked();
   ASSERT_TRUE(tag.IsMarked());
   tag.ClearMarked();
   ASSERT_FALSE(tag.IsMarked());
 }

 TEST_F(ObjectTagTest, TestRememberedBit){
   ObjectTag tag;
   ASSERT_FALSE(tag.IsRemembered());
   tag.SetRemembered();
   ASSERT_TRUE(tag.IsRemembered());
   tag.ClearRemembered();
   ASSERT_FALSE(tag.IsRemembered());
 }

 TEST_F(ObjectTagTest, TestNew){
   ObjectTag tag = ObjectTag::New();
   ASSERT_TRUE(tag.IsNew());
   ASSERT_FALSE(tag.IsOld());
   ASSERT_FALSE(tag.IsMarked());
   ASSERT_FALSE(tag.IsRemembered());
   ASSERT_EQ(tag.GetSize(), 0);
 }

 TEST_F(ObjectTagTest, TestNewWithSize){
   static const constexpr int64_t kTestObjectSize = 1 * kMB;
   ObjectTag tag = ObjectTag::NewWithSize(kTestObjectSize);
   ASSERT_TRUE(tag.IsNew());
   ASSERT_FALSE(tag.IsOld());
   ASSERT_FALSE(tag.IsMarked());
   ASSERT_FALSE(tag.IsRemembered());
   ASSERT_EQ(tag.GetSize(), kTestObjectSize);
 }

 TEST_F(ObjectTagTest, TestNewMarkedWithSize){
   static const constexpr int64_t kTestObjectSize = 1 * kMB;
   ObjectTag tag = ObjectTag::NewMarkedWithSize(kTestObjectSize);
   ASSERT_TRUE(tag.IsNew());
   ASSERT_FALSE(tag.IsOld());
   ASSERT_TRUE(tag.IsMarked());
   ASSERT_FALSE(tag.IsRemembered());
   ASSERT_EQ(tag.GetSize(), kTestObjectSize);
 }

 TEST_F(ObjectTagTest, TestNewRememberedWithSize){
   static const constexpr int64_t kTestObjectSize = 1 * kMB;
   ObjectTag tag = ObjectTag::NewRememberedWithSize(kTestObjectSize);
   ASSERT_TRUE(tag.IsNew());
   ASSERT_FALSE(tag.IsOld());
   ASSERT_FALSE(tag.IsMarked());
   ASSERT_TRUE(tag.IsRemembered());
   ASSERT_EQ(tag.GetSize(), kTestObjectSize);
 }

 TEST_F(ObjectTagTest, TestOld){
   ObjectTag tag = ObjectTag::Old();
   ASSERT_FALSE(tag.IsNew());
   ASSERT_TRUE(tag.IsOld());
   ASSERT_FALSE(tag.IsMarked());
   ASSERT_FALSE(tag.IsRemembered());
   ASSERT_EQ(tag.GetSize(), 0);
 }

 TEST_F(ObjectTagTest, TestOldWithSize){
   static const constexpr int64_t kTestObjectSize = 1 * kMB;
   ObjectTag tag = ObjectTag::OldWithSize(kTestObjectSize);
   ASSERT_FALSE(tag.IsNew());
   ASSERT_TRUE(tag.IsOld());
   ASSERT_FALSE(tag.IsMarked());
   ASSERT_FALSE(tag.IsRemembered());
   ASSERT_EQ(tag.GetSize(), kTestObjectSize);
 }

 TEST_F(ObjectTagTest, TestOldMarkedWithSize){
   static const constexpr int64_t kTestObjectSize = 1 * kMB;
   ObjectTag tag = ObjectTag::OldMarkedWithSize(kTestObjectSize);
   ASSERT_FALSE(tag.IsNew());
   ASSERT_TRUE(tag.IsOld());
   ASSERT_TRUE(tag.IsMarked());
   ASSERT_FALSE(tag.IsRemembered());
   ASSERT_EQ(tag.GetSize(), kTestObjectSize);
 }

 TEST_F(ObjectTagTest, TestOldRememberedWithSize){
   static const constexpr int64_t kTestObjectSize = 1 * kMB;
   ObjectTag tag = ObjectTag::OldRememberedWithSize(kTestObjectSize);
   ASSERT_FALSE(tag.IsNew());
   ASSERT_TRUE(tag.IsOld());
   ASSERT_FALSE(tag.IsMarked());
   ASSERT_TRUE(tag.IsRemembered());
   ASSERT_EQ(tag.GetSize(), kTestObjectSize);
 }

 class RawObjectTest : public Test{
  public:
   RawObjectTest() = default;
   ~RawObjectTest() override = default;
 };

 TEST_F(RawObjectTest, TestSizeTag){
   static const constexpr uint32_t kTestObjectSize = 142398792;

   RawObject val;
   ASSERT_EQ(val.GetPointerSize(), 0);
   val.SetPointerSize(kTestObjectSize);
   ASSERT_EQ(val.GetPointerSize(), kTestObjectSize);
 }

 TEST_F(RawObjectTest, TestObjectAddress){
   RawObject val;
   auto address = (uword)&val;
   ASSERT_EQ(val.GetObjectPointerAddress(), address + sizeof(RawObject));
 }

 TEST_F(RawObjectTest, TestForwardingAddress){
   static const constexpr uword kTestForwardingAddress = 0x47469920; // Warning: don't actually use this address for anything

   RawObject val;
   ASSERT_EQ(val.GetForwardingAddress(), 0);
   val.SetForwardingAddress(kTestForwardingAddress);
   ASSERT_TRUE(val.IsForwarding());
   ASSERT_EQ(val.GetForwardingAddress(), kTestForwardingAddress);
 }
}