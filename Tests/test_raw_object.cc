#include <gtest/gtest.h>

#include "helpers.h"
#include "poseidon/raw_object.h"

namespace poseidon{
 using namespace ::testing;

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

 TEST_F(RawObjectTest, TestNewBit){
   RawObject val;
   ASSERT_FALSE(val.IsNew());
   val.SetNewBit();
   ASSERT_TRUE(val.IsNew());
   val.ClearNewBit();
   ASSERT_FALSE(val.IsNew());
 }

 TEST_F(RawObjectTest, TestOldBit){
   RawObject val;
   ASSERT_FALSE(val.IsOld());
   val.SetOldBit();
   ASSERT_TRUE(val.IsOld());
   val.ClearOldBit();
   ASSERT_FALSE(val.IsOld());
 }

 TEST_F(RawObjectTest, TestMarkedBit){
   RawObject val;
   ASSERT_FALSE(val.IsMarked());
   val.SetMarkedBit();
   ASSERT_TRUE(val.IsMarked());
   val.ClearMarkedBit();
   ASSERT_FALSE(val.IsMarked());
 }

 TEST_F(RawObjectTest, TestRememberedBit){
   RawObject val;
   ASSERT_FALSE(val.IsRemembered());
   val.SetRememberedBit();
   ASSERT_TRUE(val.IsRemembered());
   val.ClearRememberedBit();
   ASSERT_FALSE(val.IsRemembered());
 }
}