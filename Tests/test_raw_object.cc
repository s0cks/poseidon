#include <gtest/gtest.h>

#include "helpers.h"
#include "poseidon/pointer.h"

namespace poseidon{
 using namespace ::testing;

 class RawObjectTest : public Test{
  protected:
   RawObjectTest() = default;

   static inline Pointer*
   CreateRawObject(PointerTag tag){
     return new Pointer(tag);
   }

   static inline Pointer*
   CreateNewRawObject(int64_t size){
     return new Pointer(PointerTag::New(size));
   }

   static inline Pointer*
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
   //     the size of Pointer
   //   + the size of a word
   static constexpr const int64_t kExpectedWordSize = sizeof(Pointer) + sizeof(word);
   static constexpr const int64_t kWordValue = 333;
   auto ptr = CreateNewWord(kWordValue);
   ASSERT_EQ(ptr->GetPointerSize(), kWordSize);
   ASSERT_EQ(ptr->GetTotalSize(), kExpectedWordSize);
   ASSERT_TRUE(IsWord(ptr, kWordValue));
   DLOG(INFO) << "sizeof(word) := " << Bytes(ptr->GetTotalSize());
 }

 TEST_F(RawObjectTest, TestSizeTag){
   static const constexpr uint32_t kTestObjectSize = 142398792;

   Pointer val;
   ASSERT_EQ(val.GetPointerSize(), 0);
   val.SetPointerSize(kTestObjectSize);
   ASSERT_EQ(val.GetPointerSize(), kTestObjectSize);
 }

 TEST_F(RawObjectTest, TestObjectAddress){
   Pointer val;
   auto address = (uword)&val;
   ASSERT_EQ(val.GetObjectPointerAddress(), address + sizeof(Pointer));
 }

 TEST_F(RawObjectTest, TestForwardingAddress){
   static const constexpr uword kTestForwardingAddress = 0x47469920; // Warning: don't actually use this address for anything

   Pointer val;
   ASSERT_EQ(val.GetForwardingAddress(), 0);
   val.SetForwardingAddress(kTestForwardingAddress);
   ASSERT_TRUE(val.IsForwarding());
   ASSERT_EQ(val.GetForwardingAddress(), kTestForwardingAddress);
 }
}