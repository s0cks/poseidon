#include <gtest/gtest.h>

#include "poseidon/flags.h"
#include "poseidon/type/all.h"
#include "assertions/assertions.h"
#include "freelist/test_freelist.h"
#include "freelist/is_free_ptr_to.h"
#include "freelist/mock_free_pointer.h"

namespace poseidon {
 using namespace ::testing;

 TEST_F(FreeListTest, TestConstructor) {
   //TODO: ??
 }

#define DEFINE_INSERT_FAILS_TEST(NumberOfBytes, Reason) \
  TEST_F(FreeListTest, TestInsert_WillFail_##Reason) {  \
    ASSERT_FALSE(Insert(free_list().GetStartingAddress(), NumberOfBytes)); \
  }

 DEFINE_INSERT_FAILS_TEST(-1, SizeLessThanZero);
 DEFINE_INSERT_FAILS_TEST(0, SizeEqualsZero);
 DEFINE_INSERT_FAILS_TEST(FreeList::GetMinimumSize() - 1, SizeLessThanMinimum);
 DEFINE_INSERT_FAILS_TEST(FreeList::GetMaximumSize() + 1, SizeGreaterThanMaximum);

 TEST_F(FreeListTest, TestInsert_WillFail_ExistingStartingAddress) {
   ASSERT_TRUE(Insert(free_list().GetStartingAddress(), kWordSize));
   ASSERT_FALSE(Insert(free_list().GetStartingAddress(), kWordSize));
 }

 TEST_F(FreeListTest, TestInsert_WillPass) {
   const auto r1 = Region::Subregion(test_region(), kWordSize);
   ASSERT_TRUE(Insert(r1));
   MockFreePointerVisitor visitor;
   EXPECT_CALL(visitor, VisitFreePointer(IsFreePointerTo(r1)))
    .WillOnce(Return(true));
   ASSERT_TRUE(free_list().VisitFreePointers(&visitor));
 }

 TEST_F(FreeListTest, TestInsert_WillPass_WillMergeOnInsert) {
   ASSERT_TRUE(Insert(free_list().GetStartingAddress(), kWordSize));
   ASSERT_TRUE(Insert(free_list().GetStartingAddress() + kWordSize, kWordSize));
 }

#define DEFINE_REMOVE_FAILS_TEST(NumberOfBytes, Reason) \
  TEST_F(FreeListTest, TestRemove_WillFail_##Reason) { \
    ASSERT_FALSE(Remove(Region::Subregion(test_region(), (NumberOfBytes)))); \
  }
 DEFINE_REMOVE_FAILS_TEST(-1, SizeLessThanZero);
 DEFINE_REMOVE_FAILS_TEST(0, SizeEqualsZero);
 DEFINE_REMOVE_FAILS_TEST(FreeList::GetMinimumSize() - 1, SizeLessThanMinimum);
 DEFINE_REMOVE_FAILS_TEST(FreeList::GetMaximumSize() + 1, SizeGreaterThanMaximum);

 TEST_F(FreeListTest, TestRemove_WillPass) {
   const auto r1 = Region::Subregion(test_region(), kWordSize);
   ASSERT_TRUE(Insert(r1));
   ASSERT_TRUE(Remove(r1));
 }

#define DEFINE_FIND_FAILS_TEST(NumberOfBytes, Reason) \
  TEST_F(FreeListTest, TestFindBestFit_WillFail_##Reason) { \
    FreePointer* tmp = nullptr;                    \
    ASSERT_FALSE(FindBestFit((NumberOfBytes), &tmp)); \
  }
 DEFINE_FIND_FAILS_TEST(-1, SizeLessThanZero);
 DEFINE_FIND_FAILS_TEST(0, SizeEqualsZero);
 DEFINE_FIND_FAILS_TEST(FreeList::GetMinimumSize() - 1, SizeLessThanMinimum);
 DEFINE_FIND_FAILS_TEST(FreeList::GetMaximumSize() + 1, SizeGreaterThanMaximum);

 TEST_F(FreeListTest, TestFindBestFit_WillPass_ExactSize) {
   const Region r1 = Region::Subregion(test_region(), kWordSize);
   ASSERT_TRUE(Insert(r1));
   FreePointer* fp1 = nullptr;
   ASSERT_TRUE(FindBestFit(kWordSize, &fp1));
   ASSERT_NE(fp1, nullptr);
   ASSERT_EQ(r1, (Region)*fp1);
 }

 TEST_F(FreeListTest, TestFindBestFit_WillFail_LessThanMinimumSizeRemaining) {
   const Region r1 = Region::Subregion(test_region(), kWordSize + (kWordSize - 1));
   ASSERT_TRUE(Insert(r1));
   FreePointer* fp1 = nullptr;
   ASSERT_FALSE(FindBestFit(kWordSize, &fp1));
   ASSERT_EQ(fp1, nullptr);
 }

 TEST_F(FreeListTest, TestFindBestFit_WillPass_GreaterThanEqualToMinimumSizeRemaining) {
   const Region r1 = Region::Subregion(test_region(), kWordSize * 2);
   ASSERT_TRUE(Insert(r1));
   FreePointer* fp1 = nullptr;
   ASSERT_TRUE(FindBestFit(kWordSize, &fp1));
   ASSERT_NE(fp1, nullptr);
   ASSERT_EQ(r1, (Region)*fp1);
 }

 TEST_F(FreeListTest, TestTryAllocate_OneObject) {
   MemoryRegion region(flags::GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   FreeList free_list(region);

   static const constexpr RawInt kAValue = 10;
   auto ptr = Int::TryAllocateIn<>(&free_list, kAValue);
   ASSERT_NE(ptr, nullptr);
   ASSERT_TRUE(IsInt(ptr->raw_ptr()));
   ASSERT_TRUE(IntEq(kAValue, ptr));
 }
}