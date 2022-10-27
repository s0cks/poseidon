#include <gtest/gtest.h>

#include "helpers.h"
#include "poseidon/raw_object.h"

namespace poseidon{
 using namespace ::testing;

 class RawObjectTest : public Test{
  protected:
   RawObjectTest() = default;

   static inline RawObject*
   CreateRawObject(ObjectTag tag){
     return new RawObject(tag);
   }

   static inline RawObject*
   CreateNewRawObject(int64_t size){
     return new RawObject(ObjectTag::New(size));
   }

   static inline RawObject*
   CreateNewWord(word value){
     auto val = CreateNewRawObject(sizeof(word));
     *((word*)val->GetPointer()) = value;
     return val;
   }
  public:
   ~RawObjectTest() override = default;
 };

 TEST_F(RawObjectTest, TestWordSize){
   // words should be equal to:
   //     the size of RawObject
   //   + the size of a word
   static constexpr const int64_t kExpectedWordSize = sizeof(RawObject) + sizeof(word);
   static constexpr const int64_t kWordValue = 333;
   auto ptr = CreateNewWord(kWordValue);
   ASSERT_EQ(ptr->GetPointerSize(), kWordSize);
   ASSERT_EQ(ptr->GetTotalSize(), kExpectedWordSize);
   ASSERT_TRUE(IsWord(ptr, kWordValue));
   DLOG(INFO) << "sizeof(word) := " << Bytes(ptr->GetTotalSize());
 }

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