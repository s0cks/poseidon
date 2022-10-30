#include <gtest/gtest.h>

#include "helpers.h"
#include "poseidon/flags.h"
#include "poseidon/heap/freelist.h"

namespace poseidon {
 using namespace ::testing;

 class IsFreePointerToMatcher {
  protected:
   uword start_;
   ObjectSize size_;
  public:
   using is_gtest_matcher = void;

   explicit IsFreePointerToMatcher(const uword start,
                                   const ObjectSize size):
       start_(start),
       size_(size) {
   }
   ~IsFreePointerToMatcher() = default;

   bool MatchAndExplain(FreeObject* actual, std::ostream*) const {
     return actual->GetStartingAddress() == start_ &&
         actual->GetSize() == size_;
   }

   void DescribeTo(std::ostream* stream) const {
     (*stream) << "free pointer points to " << ((void*) start_) << " (" << Bytes(size_) << ")";
   }

   void DescribeNegationTo(std::ostream* stream) const {
     (*stream) << "free pointer does not point to " << ((void*) start_) << " (" << Bytes(size_) << ")";
   }
 };

 Matcher<FreeObject*> IsFreePointerTo(const uword start, const int64_t size) {
   return IsFreePointerToMatcher(start, size);
 }

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

 class FreeListPrinter : public FreeObjectVisitor {
  public:
   FreeListPrinter() = default;
   ~FreeListPrinter() override = default;

   bool Visit(FreeObject* ptr) override {
     DLOG(INFO) << (*ptr);
     return true;
   }
 };

 static inline bool
 Print(FreeList& free_list) {
   FreeListPrinter printer;
   return free_list.VisitFreePointers(&printer);
 }

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

 TEST_F(FreeListTest, TestInsert_WillFail_StartingAddressIsNotContained) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   FreeList free_list(region);
   ASSERT_FALSE(free_list.Insert(region.GetStartingAddress() -1, region.GetSize()));
 }

 TEST_F(FreeListTest, TestInsert_WillFail_SizeLessThanZero) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   FreeList free_list(region);
   ASSERT_FALSE(free_list.Insert(region.GetStartingAddress(), -1));
 }

 TEST_F(FreeListTest, TestInsert_WillFail_SizeEqualsZero) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   FreeList free_list(region);
   ASSERT_FALSE(free_list.Insert(region.GetStartingAddress(), 0));
 }

 TEST_F(FreeListTest, TestInsert_WillFail_SizeGreaterThanMax) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   FreeList free_list(region);
   ASSERT_FALSE(free_list.Insert(region.GetStartingAddress(), region.GetSize() + 1));
 }

 TEST_F(FreeListTest, TestInsert_WillFail_SizeLessThanWordSize) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   FreeList free_list(region);
   ASSERT_FALSE(free_list.Insert(region.GetStartingAddress(), kWordSize - 1));
 }

 TEST_F(FreeListTest, TestInsert_WillFail_DuplicateStartingAddress) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   FreeList free_list(region);
   ASSERT_FALSE(free_list.Insert(region.GetStartingAddress(), region.GetSize()));
 }

 TEST_F(FreeListTest, TestInsert_WillPass_SizeEqualToWordSize) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   FreeList free_list(region);
   ASSERT_TRUE(free_list.Remove(region.GetStartingAddress(), region.GetSize()));

   ASSERT_TRUE(free_list.Insert(region.GetStartingAddress(), kWordSize));
 }

 TEST_F(FreeListTest, TestInsert_WillPass_WillMergeOnInsert) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   FreeList free_list(region);
   ASSERT_TRUE(free_list.Remove(region.GetStartingAddress(), region.GetSize()));

   ASSERT_TRUE(free_list.Insert(region.GetStartingAddress(), kWordSize + sizeof(RawObject)));
   ASSERT_TRUE(free_list.Insert(region.GetStartingAddress() + (kWordSize + sizeof(RawObject)), kWordSize + sizeof(RawObject)));

   MockFreeListNodeVisitor visitor;
   EXPECT_CALL(visitor, Visit(IsFreePointerTo(region.GetStartingAddress(), (kWordSize + sizeof(RawObject)) * 2)))
    .Times(1)
    .WillRepeatedly(Return(true));
   ASSERT_NO_FATAL_FAILURE(free_list.VisitFreePointers(&visitor));
 }

 TEST_F(FreeListTest, TestRemove_WillPass) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   FreeList free_list(region);
   ASSERT_TRUE(free_list.Remove(region.GetStartingAddress(), region.GetSize()));
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
   auto ptr = free_list.FindFirstFit(-1);
   ASSERT_EQ(ptr, nullptr);
 }

 TEST_F(FreeListTest, TestFind_WillFail_SizeEqualsZero) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   FreeList free_list(region);
   auto ptr = free_list.FindFirstFit(0);
   ASSERT_EQ(ptr, nullptr);
 }

 TEST_F(FreeListTest, TestFind_WillPass_SizeExact) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   FreeList free_list(region);
   auto ptr = free_list.FindFirstFit(GetOldZoneSize());
   ASSERT_NE(ptr, nullptr);
   ASSERT_EQ(ptr->GetStartingAddress(), region.GetStartingAddress());
   ASSERT_EQ(ptr->GetSize(), region.GetSize());
 }

 TEST_F(FreeListTest, TestFind_WillPass_SizeLessThanExact) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   FreeList free_list(region);
   auto ptr = free_list.FindFirstFit(kWordSize);
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

   static const int64_t kRemainingBytes = free_list.GetSize() - (kWordSize + sizeof(RawObject));
   DLOG(INFO) << "remaining bytes: " << Bytes(kRemainingBytes);
   auto free_ptr = free_list.FindFirstFit(kRemainingBytes);
   ASSERT_NE(free_ptr, nullptr);
 }

 TEST_F(FreeListTest, TestVisit_WillPass) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   FreeList free_list(region);

   MockFreeListNodeVisitor visitor;
   EXPECT_CALL(visitor, Visit(IsFreePointerTo(region.GetStartingAddress(), region.GetSize())))
    .Times(1)
    .WillRepeatedly(Return(true));
   ASSERT_TRUE(free_list.VisitFreePointers(&visitor));
 }

 TEST_F(FreeListTest, TestVisit_WillPass_MultipleNoncontiguousFree) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   FreeList free_list(region);
   ASSERT_TRUE(free_list.Remove(region.GetStartingAddress(), region.GetSize()));

   const auto f1_start = region.GetStartingAddress();
   const auto f1_size = 1 * kKB;
   ASSERT_TRUE(free_list.Insert(f1_start, f1_size));

   const auto f2_start = f1_start + f1_size;
   const auto f2_size = 1 * kMB;
   // dont insert f2 use it f3's calculation

   const auto f3_start = f2_start + f2_size;
   const auto f3_size = 1 * kKB;
   ASSERT_TRUE(free_list.Insert(f3_start, f3_size));

   MockFreeListNodeVisitor visitor;
   EXPECT_CALL(visitor, Visit(IsFreePointerTo(f1_start, f1_size)))
    .Times(1)
    .WillRepeatedly(Return(true));
   EXPECT_CALL(visitor, Visit(IsFreePointerTo(f3_start, f3_size)))
    .Times(1)
    .WillRepeatedly(Return(true));
   ASSERT_TRUE(free_list.VisitFreePointers(&visitor));
 }
}