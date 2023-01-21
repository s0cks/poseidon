#include <gtest/gtest.h>

#include "poseidon/flags.h"
#include "poseidon/type/all.h"
#include "assertions/assertions.h"
#include "freelist/is_free_ptr_to.h"
#include "freelist/mock_free_pointer.h"
#include "poseidon/freelist/freelist.h"
#include "poseidon/freelist/freelist_printer.h"

namespace poseidon {
 using namespace ::testing;

 class FreeListTest : public Test {
  private:
   MemoryRegion test_region_;
   FreeList free_list_;
  protected:
   FreeListTest():
    Test(),
    test_region_(flags::GetOldZoneSize()),
    free_list_(test_region_, false) {
   }

   inline MemoryRegion& test_region() {
     return test_region_;
   }

   inline FreeList& free_list() {
     return free_list_;
   }

   inline bool Insert(const Region& region) {
     return free_list().Insert(region);
   }

   inline bool Insert(const uword start, const RegionSize size) {
     return Insert({ start, size });
   }

   inline bool FindBestFit(const RegionSize size, FreePointer** result) {
     return free_list().FindBestFit(size, result);
   }

   inline bool
   Remove(const Region& region) {
     return free_list().Remove(region);
   }

   static inline uword
   TryAllocateBytes(FreeList& free_list, const ObjectSize size) {
     return free_list.TryAllocateBytes(size);
   }

   static inline Pointer*
   TryAllocateWord(FreeList& free_list, word value) {
     auto address = TryAllocateBytes(free_list, kWordSize);
     if (address == UNALLOCATED)
       return nullptr;
     auto ptr = (Pointer*)address;
     (*((word*)ptr->GetObjectPointerAddress())) = value;
     return ptr;
   }

   static inline Pointer*
   TryAllocateMarkedWord(FreeList& free_list, word value) {
     auto address = TryAllocateBytes(free_list, kWordSize);
     if (address == UNALLOCATED)
       return nullptr;
     auto ptr = (Pointer*)address;
     ptr->SetMarkedBit();
     (*((word*)ptr->GetObjectPointerAddress())) = value;
     return ptr;
   }
  public:
   ~FreeListTest() override = default;

   void SetUp() override {
     ASSERT_TRUE(test_region_.Protect(MemoryRegion::kReadWrite));
     ASSERT_NO_FATAL_FAILURE(test_region_.ClearRegion());
     ASSERT_NO_FATAL_FAILURE(free_list_.ClearFreeList());
#ifdef PSDN_DEBUG
     DLOG(INFO) << "Freelist (Before):";
     FreeListPrinter::Print(&free_list());
#endif //PSDN_DEBUG
   }

   void TearDown() override {
#ifdef PSDN_DEBUG
     DLOG(INFO) << "Freelist (After):";
     FreeListPrinter::Print(&free_list());
#endif //PSDN_DEBUG
     ASSERT_TRUE(test_region_.Protect(MemoryRegion::kReadOnly));
   }
 };

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