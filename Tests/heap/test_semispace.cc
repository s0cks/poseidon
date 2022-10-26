#include "helpers.h"
#include "helpers/assertions.h"

namespace poseidon{
#define UNALLOCATED 0 //TODO: cleanup

 using namespace ::testing;

 class SemispaceTest : public Test {
  protected:
   SemispaceTest() = default;

   static inline uword
   TryAllocateBytes(Semispace& semispace, const ObjectSize size) {
     return semispace.TryAllocate(size);
   }

   static inline RawObject*
   TryAllocateWord(Semispace& semispace, const word value) {
     auto address = TryAllocateBytes(semispace, kWordSize);
     if(address == 0)
       return nullptr;
     auto ptr = (RawObject*)address;
     (*((uword*)ptr->GetObjectPointerAddress())) = value;
     return ptr;
   }

   static inline RawObject*
   TryAllocateMarkedWord(Semispace& semispace, const word value) {
     auto address = TryAllocateBytes(semispace, kWordSize);
     if(address == 0)
       return nullptr;
     auto ptr = (RawObject*)address;
     ptr->SetMarkedBit();
     (*((uword*)ptr->GetObjectPointerAddress())) = value;
     return ptr;
   }
  public:
   ~SemispaceTest() override = default;
 };

 TEST_F(SemispaceTest, TestConstructor) {
   MemoryRegion region(GetNewZoneSemispaceSize());
   Semispace semispace(region);
   ASSERT_EQ((const Region&) semispace, (const Region&)region);
   ASSERT_EQ(semispace.GetCurrentAddress(), semispace.GetStartingAddress());
   ASSERT_EQ(semispace.GetNumberOfBytesAllocated(), 0);
   ASSERT_EQ(semispace.GetNumberOfBytesRemaining(), semispace.GetSize());
 }

 TEST_F(SemispaceTest, TestEquals) {

 }

 TEST_F(SemispaceTest, TestCopyConstructor) {

 }

 TEST_F(SemispaceTest, TestTryAllocateWillFailEqualToZero) {
   MemoryRegion region(GetNewZoneSemispaceSize());
   Semispace semispace(region);
   auto ptr = TryAllocateBytes(semispace, 0);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(SemispaceTest, TestTryAllocateWillFailLessThanZero) {
   MemoryRegion region(GetNewZoneSemispaceSize());
   Semispace semispace(region);
   auto ptr = TryAllocateBytes(semispace, -1);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(SemispaceTest, TestTryAllocateWillFailEqualToSize) {
   MemoryRegion region(GetNewZoneSemispaceSize());
   Semispace semispace(region);
   auto ptr = TryAllocateBytes(semispace, GetNewZoneSemispaceSize());
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(SemispaceTest, TestTryAllocateWillFailGreaterThanSize) {
   MemoryRegion region(GetNewZoneSemispaceSize());
   Semispace semispace(region);
   auto ptr = TryAllocateBytes(semispace, GetNewZoneSemispaceSize() + 1);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(SemispaceTest, TestTryAllocate){
   MemoryRegion region(GetNewZoneSemispaceSize());
   Semispace semispace(region);

   static const constexpr word kDefaultWordValue = 42;
   auto ptr = TryAllocateWord(semispace, kDefaultWordValue);
   ASSERT_TRUE(IsAllocated(ptr));
   ASSERT_TRUE(IsNew(ptr));
   ASSERT_FALSE(IsOld(ptr));
   ASSERT_FALSE(IsMarked(ptr));
   ASSERT_FALSE(IsRemembered(ptr));
   ASSERT_FALSE(IsForwarding(ptr));
   ASSERT_TRUE(IsWord(ptr, kDefaultWordValue));
 }

 TEST_F(SemispaceTest, TestVisitPointers){
   MemoryRegion region(GetNewZoneSemispaceSize());
   Semispace semispace(region);

   static const constexpr int64_t kNumberOfPointers = 3;
   for(auto idx = 0; idx < kNumberOfPointers; idx++){
     auto ptr = TryAllocateWord(semispace, idx);
     ASSERT_TRUE(IsAllocated(ptr));
     ASSERT_TRUE(IsNew(ptr));
     ASSERT_FALSE(IsOld(ptr));
     ASSERT_FALSE(IsMarked(ptr));
     ASSERT_FALSE(IsRemembered(ptr));
     ASSERT_FALSE(IsForwarding(ptr));
     ASSERT_TRUE(IsWord(ptr, idx));
   }

   MockRawObjectVisitor visitor;
   EXPECT_CALL(visitor, Visit)
    .Times(kNumberOfPointers);
   ASSERT_NO_FATAL_FAILURE(semispace.VisitPointers(&visitor));
 }

 TEST_F(SemispaceTest, TestVisitMarkedPointers){
   MemoryRegion region(GetNewZoneSemispaceSize());
   Semispace semispace(region);

   static const constexpr int64_t kNumberOfUnmarkedPointers = 1;
   static const constexpr int64_t kNumberOfMarkedPointers = 3;

   for(auto idx = 0; idx < kNumberOfUnmarkedPointers; idx++){
     auto ptr = TryAllocateWord(semispace, idx);
     ASSERT_TRUE(IsAllocated(ptr));
     ASSERT_TRUE(IsNew(ptr));
     ASSERT_FALSE(IsOld(ptr));
     ASSERT_FALSE(IsMarked(ptr));
     ASSERT_FALSE(IsRemembered(ptr));
     ASSERT_FALSE(IsForwarding(ptr));
     ASSERT_TRUE(IsWord(ptr, idx));
   }

   for(auto idx = 0; idx < kNumberOfMarkedPointers; idx++){
     auto ptr = TryAllocateMarkedWord(semispace, idx);
     ASSERT_TRUE(IsAllocated(ptr));
     ASSERT_TRUE(IsNew(ptr));
     ASSERT_FALSE(IsOld(ptr));
     ASSERT_TRUE(IsMarked(ptr));
     ASSERT_FALSE(IsRemembered(ptr));
     ASSERT_FALSE(IsForwarding(ptr));
     ASSERT_TRUE(IsWord(ptr, idx));
   }

   MockRawObjectVisitor visitor;
   EXPECT_CALL(visitor, Visit)
    .Times(kNumberOfMarkedPointers);
   ASSERT_NO_FATAL_FAILURE(semispace.VisitMarkedPointers(&visitor));
 }
}