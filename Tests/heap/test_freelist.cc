#include <gtest/gtest.h>

#include "helpers.h"
#include "poseidon/flags.h"
#include "poseidon/heap/freelist.h"

namespace poseidon {
#define UNALLOCATED 0 //TODO: cleanup

 using namespace ::testing;

 class FreeListTest : public Test {
  protected:
   FreeListTest() = default;

   static inline uword
   TryAllocateBytes(FreeList& free_list, const ObjectSize size) {
     return free_list.TryAllocate(size);
   }

   static inline RawObject*
   TryAllocateWord(FreeList& free_list, word value) {
     auto address = TryAllocateBytes(free_list, kWordSize);
     if (address == UNALLOCATED)
       return nullptr;
     auto ptr = (RawObject*)address;
     (*((word*)ptr->GetObjectPointerAddress())) = value;
     return ptr;
   }

   static inline RawObject*
   TryAllocateMarkedWord(FreeList& free_list, word value) {
     auto address = TryAllocateBytes(free_list, kWordSize);
     if (address == UNALLOCATED)
       return nullptr;
     auto ptr = (RawObject*)address;
     ptr->SetMarkedBit();
     (*((word*)ptr->GetObjectPointerAddress())) = value;
     return ptr;
   }
  public:
   ~FreeListTest() override = default;
 };

 TEST_F(FreeListTest, TestConstructor) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   FreeList free_list(region);
   //TODO: ??
 }

 //TODO: add Equals & NotEquals tests

 TEST_F(FreeListTest, TestTryAllocate_WillFail_SizeLessThanZero) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   FreeList free_list(region);
   auto ptr = TryAllocateBytes(free_list, -1);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(FreeListTest, TestTryAllocate_WillFail_SizeEqualsZero) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   FreeList free_list(region);
   auto ptr = TryAllocateBytes(free_list, 0);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(FreeListTest, TestTryAllocate_WillFail_SizeGreaterThanAvailable) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   FreeList free_list(region);
   auto ptr = TryAllocateBytes(free_list, GetOldZoneSize() + 1);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(FreeListTest, TestRemove_WillPass) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   FreeList free_list(region);
   ASSERT_TRUE(free_list.Remove(region.GetStartingAddress(), region.GetSize()));
 }

 TEST_F(FreeListTest, TestRemove_WillFail_NoMatchingStartingAddress) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   FreeList free_list(region);
   ASSERT_FALSE(free_list.Remove(region.GetStartingAddress() + kWordSize, region.GetSize()));
 }

 TEST_F(FreeListTest, TestRemove_WillFail_NoMatchingSize) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   FreeList free_list(region);
   ASSERT_FALSE(free_list.Remove(region.GetStartingAddress(), region.GetSize() - kWordSize));
 }

 TEST_F(FreeListTest, TestFind_WillFail_SizeLessThanZero) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   FreeList free_list(region);
   auto ptr = free_list.FindBestFit(-1);
   ASSERT_EQ(ptr, nullptr);
 }

 TEST_F(FreeListTest, TestFind_WillFail_SizeEqualsZero) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   FreeList free_list(region);
   auto ptr = free_list.FindBestFit(0);
   ASSERT_EQ(ptr, nullptr);
 }

 TEST_F(FreeListTest, TestFind_WillPass_SizeExact) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   FreeList free_list(region);
   auto ptr = free_list.FindBestFit(GetOldZoneSize());
   ASSERT_NE(ptr, nullptr);
   ASSERT_EQ(ptr->GetStartingAddress(), region.GetStartingAddress());
   ASSERT_EQ(ptr->GetSize(), region.GetSize());
 }

 TEST_F(FreeListTest, TestFind_WillPass_SizeLessThanExact) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   FreeList free_list(region);
   auto ptr = free_list.FindBestFit(kWordSize);
   ASSERT_NE(ptr, nullptr);
   ASSERT_EQ(ptr->GetStartingAddress(), region.GetStartingAddress());
   ASSERT_EQ(ptr->GetSize(), region.GetSize());
 }

 TEST_F(FreeListTest, TestTryAllocate_OneObject) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   FreeList free_list(region);

   static const constexpr int64_t kDefaultWordValue = 100;
   auto ptr = TryAllocateWord(free_list, kDefaultWordValue);
   ASSERT_TRUE(IsAllocated(ptr));
   ASSERT_FALSE(IsNew(ptr));
   ASSERT_TRUE(IsOld(ptr));
   ASSERT_FALSE(IsMarked(ptr));
   ASSERT_FALSE(IsRemembered(ptr));
   ASSERT_FALSE(IsForwarding(ptr));
   ASSERT_TRUE(IsWord(ptr, kDefaultWordValue));
 }

//TODO:
// TEST_F(FreeListTest, TestInsert_WillPass) {
//   MemoryRegion region(GetOldZoneSize());
//   FreeList free_list(region);
//   static const constexpr int64_t kAValue = 100;
//   auto a = TryAllocateWord(free_list, kAValue);
//   ASSERT_TRUE(IsAllocated(a));
//   ASSERT_FALSE(IsNew(a));
//   ASSERT_TRUE(IsOld(a));
//   ASSERT_FALSE(IsMarked(a));
//   ASSERT_FALSE(IsRemembered(a));
//   ASSERT_FALSE(IsForwarding(a));
//   ASSERT_TRUE(IsWord(a, kAValue));
//   ASSERT_TRUE(Insert(free_list, a));
// }

//TODO:
// TEST_F(FreeListTest, TestVisitNodes){
//   MemoryRegion region(GetOldZoneSize());
//   FreeList free_list(region);
//   static const constexpr int64_t kNumberOfPointers = 10;
//   for(auto idx = 0; idx < kNumberOfPointers; idx++) {
//     auto a = TryAllocateWord(free_list, idx);
//     ASSERT_TRUE(IsAllocated(a));
//     ASSERT_FALSE(IsNew(a));
//     ASSERT_TRUE(IsOld(a));
//     ASSERT_FALSE(IsMarked(a));
//     ASSERT_FALSE(IsRemembered(a));
//     ASSERT_FALSE(IsForwarding(a));
//     ASSERT_TRUE(IsWord(a, idx));
//     ASSERT_TRUE(Insert(free_list, a));
//   }
//   MockFreeListNodeVisitor visitor;
//   EXPECT_CALL(visitor, Visit)
//     .Times(kNumberOfPointers + 1);
//   ASSERT_TRUE(free_list.VisitFreeNodes(&visitor));
// }
}