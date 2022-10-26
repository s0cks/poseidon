#include <gtest/gtest.h>

#include "helpers.h"
#include "poseidon/flags.h"
#include "poseidon/heap/freelist.h"
#include "poseidon/heap/freelist_printer.h"

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

   static inline bool
   Insert(FreeList& free_list, const uword starting_address, const ObjectSize size) {
     return free_list.Insert(starting_address, size);
   }

   static inline bool
   Insert(FreeList& free_list, RawObject* ptr) {
     return Insert(free_list, ptr->GetStartingAddress(), ptr->GetSize());
   }
  public:
   ~FreeListTest() override = default;
 };

 TEST_F(FreeListTest, TestConstructor) {
   MemoryRegion region(GetOldZoneSize());
   FreeList free_list(region);
   ASSERT_EQ(free_list.GetNumberOfNodes(), 1);

   // check head
   auto head = free_list.GetHead();
   ASSERT_EQ(head->GetStartingAddress(), region.GetStartingAddress());
   ASSERT_EQ(head->GetSize(), region.GetSize());
   ASSERT_EQ(head->GetNextAddress(), region.GetEndingAddress());
 }

 TEST_F(FreeListTest, TestEquals) {
   MemoryRegion region(GetOldZoneSize());
   FreeList a(region);
   FreeList b(region);
   ASSERT_EQ(a, b);
 }

 TEST_F(FreeListTest, TestTryAllocate_WillFail_SizeLessThanZero) {
   MemoryRegion region(GetOldZoneSize());
   FreeList free_list(region);
   auto ptr = TryAllocateBytes(free_list, -1);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(FreeListTest, TestTryAllocate_WillFail_SizeEqualsZero) {
   MemoryRegion region(GetOldZoneSize());
   FreeList free_list(region);
   auto ptr = TryAllocateBytes(free_list, 0);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(FreeListTest, TestTryAllocate_WillFail_SizeGreaterThanAvailable) {
   MemoryRegion region(GetOldZoneSize());
   FreeList free_list(region);
   auto ptr = TryAllocateBytes(free_list, GetOldZoneSize() + 1);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(FreeListTest, TestTryAllocate_OneObject) {
   MemoryRegion region(GetOldZoneSize());
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

   auto new_head = free_list.GetHead();
   ASSERT_TRUE(new_head);
   ASSERT_EQ(new_head->GetStartingAddress(), region.GetStartingAddress() + ptr->GetTotalSize());
   ASSERT_EQ(new_head->GetSize(), region.GetSize() - ptr->GetTotalSize());
   ASSERT_EQ(new_head->GetNextAddress(), region.GetStartingAddress() + ptr->GetTotalSize() + new_head->GetSize());
 }

 TEST_F(FreeListTest, TestTryAllocate_MultipleObjects) {
   MemoryRegion region(GetOldZoneSize());
   FreeList free_list(region);

   static const constexpr int64_t kAValue = 100;
   auto a = TryAllocateWord(free_list, kAValue);
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_FALSE(IsNew(a));
   ASSERT_TRUE(IsOld(a));
   ASSERT_FALSE(IsMarked(a));
   ASSERT_FALSE(IsRemembered(a));
   ASSERT_FALSE(IsForwarding(a));
   ASSERT_TRUE(IsWord(a, kAValue));

   static const constexpr int64_t kBValue = 400;
   auto b = TryAllocateWord(free_list, kBValue);
   ASSERT_TRUE(IsAllocated(b));
   ASSERT_FALSE(IsNew(b));
   ASSERT_TRUE(IsOld(b));
   ASSERT_FALSE(IsMarked(b));
   ASSERT_FALSE(IsRemembered(b));
   ASSERT_FALSE(IsForwarding(b));
   ASSERT_TRUE(IsWord(b, kBValue));

   auto new_head = free_list.GetHead();
   ASSERT_TRUE(new_head);
   ASSERT_EQ(new_head->GetStartingAddress(), region.GetStartingAddress() + a->GetTotalSize() + b->GetTotalSize());
   ASSERT_EQ(new_head->GetSize(), region.GetSize() - a->GetTotalSize() - b->GetTotalSize());
 }

 TEST_F(FreeListTest, TestInsert_WillFail_SizeLessThanZero) {
   MemoryRegion region(GetOldZoneSize());
   FreeList free_list(region);
   ASSERT_FALSE(Insert(free_list, region.GetStartingAddress(), -1));
 }

 TEST_F(FreeListTest, TestInsert_WillFail_SizeEqualsZero) {
   MemoryRegion region(GetOldZoneSize());
   FreeList free_list(region);
   ASSERT_FALSE(Insert(free_list, region.GetStartingAddress(), 0));
 }

 TEST_F(FreeListTest, TestInsert_WillFail_SizeGreaterThanFreeList) {
   MemoryRegion region(GetOldZoneSize());
   FreeList free_list(region);
   ASSERT_FALSE(Insert(free_list, region.GetStartingAddress(), region.GetSize() + 1));
 }

 TEST_F(FreeListTest, TestInsert_WillPass_SizeEqualsFreeList) {
   MemoryRegion region(GetOldZoneSize());
   FreeList free_list(region);

   auto a = TryAllocateBytes(free_list, free_list.GetHead()->GetSize() - sizeof(RawObject));
   ASSERT_NE(a, UNALLOCATED);
   ASSERT_TRUE(Insert(free_list, a, GetOldZoneSize()));
 }

 TEST_F(FreeListTest, TestInsert_WillPass) {
   MemoryRegion region(GetOldZoneSize());
   FreeList free_list(region);

   static const constexpr int64_t kAValue = 100;
   auto a = TryAllocateWord(free_list, kAValue);
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_FALSE(IsNew(a));
   ASSERT_TRUE(IsOld(a));
   ASSERT_FALSE(IsMarked(a));
   ASSERT_FALSE(IsRemembered(a));
   ASSERT_FALSE(IsForwarding(a));
   ASSERT_TRUE(IsWord(a, kAValue));
   ASSERT_TRUE(Insert(free_list, a));
 }

 TEST_F(FreeListTest, TestVisitNodes){
   MemoryRegion region(GetOldZoneSize());
   FreeList free_list(region);

   static const constexpr int64_t kNumberOfPointers = 10;
   for(auto idx = 0; idx < kNumberOfPointers; idx++) {
     auto a = TryAllocateWord(free_list, idx);
     ASSERT_TRUE(IsAllocated(a));
     ASSERT_FALSE(IsNew(a));
     ASSERT_TRUE(IsOld(a));
     ASSERT_FALSE(IsMarked(a));
     ASSERT_FALSE(IsRemembered(a));
     ASSERT_FALSE(IsForwarding(a));
     ASSERT_TRUE(IsWord(a, idx));
     ASSERT_TRUE(Insert(free_list, a));
   }

   FreeListPrinter::Print(free_list);

   MockFreeListNodeVisitor visitor;
   EXPECT_CALL(visitor, Visit)
     .Times(kNumberOfPointers + 1);
   ASSERT_TRUE(free_list.VisitFreeNodes(&visitor));
 }
}