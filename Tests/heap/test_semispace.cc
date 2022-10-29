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
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   Semispace semispace(region);
   ASSERT_EQ(semispace.GetStartingAddress(), region.GetStartingAddress());
   ASSERT_TRUE(semispace.IsEmpty());
   ASSERT_EQ(semispace.GetCurrentAddress(), semispace.GetCurrentAddress());
   ASSERT_EQ(semispace.GetSize(), region.GetSize());
 }

 TEST_F(SemispaceTest, TestEquals) {
   MemoryRegion region(GetNewZoneSemispaceSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));

   Semispace a(region);
   Semispace b(region);
   ASSERT_TRUE(a == b);
 }

 TEST_F(SemispaceTest, TestSwap) {
   // a == r1
   MemoryRegion r1(GetNewZoneSemispaceSize());
   ASSERT_TRUE(r1.Protect(MemoryRegion::kReadWrite));
   Semispace a(r1);
   ASSERT_EQ(a.GetStartingAddress(), r1.GetStartingAddress());
   ASSERT_EQ(a.GetCurrentAddress(), r1.GetStartingAddress());
   ASSERT_EQ(a.GetSize(), r1.GetSize());

   static constexpr const word kAValue = 1034;
   auto ptr = TryAllocateWord(a, kAValue);
   ASSERT_TRUE(IsAllocated(ptr));
   ASSERT_TRUE(IsNewWord(ptr, kAValue));
   ASSERT_TRUE(a.Contains(*ptr));

   // b == r2
   MemoryRegion r2(GetNewZoneSemispaceSize());
   ASSERT_TRUE(r2.Protect(MemoryRegion::kReadWrite));
   Semispace b(r2);
   ASSERT_EQ(b.GetStartingAddress(), r2.GetStartingAddress());
   ASSERT_EQ(b.GetCurrentAddress(), r2.GetStartingAddress());
   ASSERT_EQ(b.GetSize(), r2.GetSize());
   ASSERT_FALSE(b.Contains(*ptr));

   ASSERT_NE(a, b);
   ASSERT_NO_FATAL_FAILURE(std::swap(a, b));
   ASSERT_TRUE(IsAllocated(ptr));
   ASSERT_TRUE(IsNewWord(ptr, kAValue));

   // a == r2
   ASSERT_EQ(a.GetStartingAddress(), r2.GetStartingAddress());
   ASSERT_EQ(a.GetCurrentAddress(), r2.GetStartingAddress());
   ASSERT_EQ(a.GetSize(), r2.GetSize());
   ASSERT_FALSE(a.Contains(*ptr));

   // b == r1
   ASSERT_EQ(b.GetStartingAddress(), r1.GetStartingAddress());
   ASSERT_EQ(b.GetCurrentAddress(), r1.GetStartingAddress() + ptr->GetTotalSize());
   ASSERT_EQ(b.GetSize(), r1.GetSize());
   ASSERT_TRUE(b.Contains(*ptr));
 }

 TEST_F(SemispaceTest, TestTryAllocateWillFailEqualToZero) {
   MemoryRegion region(GetNewZoneSemispaceSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   Semispace semispace(region);
   auto ptr = TryAllocateBytes(semispace, 0);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(SemispaceTest, TestTryAllocateWillFailLessThanZero) {
   MemoryRegion region(GetNewZoneSemispaceSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   Semispace semispace(region);
   auto ptr = TryAllocateBytes(semispace, -1);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(SemispaceTest, TestTryAllocateWillFailEqualToSize) {
   MemoryRegion region(GetNewZoneSemispaceSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   Semispace semispace(region);
   auto ptr = TryAllocateBytes(semispace, GetNewZoneSemispaceSize());
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(SemispaceTest, TestTryAllocateWillFailGreaterThanSize) {
   MemoryRegion region(GetNewZoneSemispaceSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   Semispace semispace(region);
   auto ptr = TryAllocateBytes(semispace, GetNewZoneSemispaceSize() + 1);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(SemispaceTest, TestTryAllocate){
   MemoryRegion region(GetNewZoneSemispaceSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
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
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
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
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
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