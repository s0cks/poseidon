#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "poseidon/heap/new_page.h"

#include "helpers.h"

namespace poseidon {
#define UNALLOCATED 0 //TODO: cleanup

 using namespace ::testing;

 class NewPageTest : public Test {
  protected:
   NewPageTest() = default;

   static inline uword
   TryAllocateBytes(NewPage& page, const ObjectSize size) {
     return page.TryAllocate(size);
   }

   static inline RawObject*
   TryAllocateWord(NewPage& page, word value) {
     auto address = TryAllocateBytes(page, kWordSize);
     if (address == UNALLOCATED)
       return nullptr;
     auto ptr = (RawObject*)address;
     (*((word*)ptr->GetObjectPointerAddress())) = value;
     return ptr;
   }

   static inline RawObject*
   TryAllocateMarkedWord(NewPage& page, word value) {
     auto address = TryAllocateBytes(page, kWordSize);
     if (address == UNALLOCATED)
       return nullptr;
     auto ptr = (RawObject*)address;
     ptr->SetMarkedBit();
     (*((word*)ptr->GetObjectPointerAddress())) = value;
     return ptr;
   }

   static inline void
   SetTag(NewPage& page, const PageTag& tag) {
     page.SetTag(tag.raw()); //TODO: cleanup
   }
  public:
   ~NewPageTest() override = default;
 };

 TEST_F(NewPageTest, TestConstructor) {
   static const constexpr int64_t kPageIndex = 0;
   MemoryRegion region(NewPage::CalculateNewPageSize());
   NewPage page(kPageIndex, region);
   ASSERT_EQ((const Region&)page, (const Region&)region);
   ASSERT_EQ(page.index(), kPageIndex);
   ASSERT_TRUE(page.IsEmpty());
   ASSERT_FALSE(page.marked());
 }

 TEST_F(NewPageTest, TestEquals) {
   static const constexpr int64_t kPageIndex = 0;
   MemoryRegion region(GetNewPageSize());
   NewPage a(kPageIndex, region);
   NewPage b(kPageIndex, region);
   ASSERT_EQ(a, b);
 }

 TEST_F(NewPageTest, TestNotEquals) {
   MemoryRegion region(GetNewPageSize());
   NewPage a(0, region);
   NewPage b(1, region);
   ASSERT_NE(a, b);
 }

 TEST_F(NewPageTest, TestTryAllocateWillFailEqualToZero) {
   MemoryRegion region(GetNewPageSize());
   NewPage page(0, region);
   auto ptr = TryAllocateBytes(page, 0);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(NewPageTest, TestTryAllocateWillFailLessThanZero) {
   MemoryRegion region(GetNewPageSize());
   NewPage page(0, region);
   auto ptr = TryAllocateBytes(page, -1);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(NewPageTest, TestTryAllocateWillFailEqualToPageSize) {
   MemoryRegion region(GetNewPageSize());
   NewPage page(0, region);
   auto ptr = TryAllocateBytes(page, GetNewPageSize());
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(NewPageTest, TestTryAllocateWillFailGreaterThanPageSize) {
   MemoryRegion region(GetNewPageSize());
   NewPage page(0, region);
   auto ptr = TryAllocateBytes(page, GetNewPageSize() + 1);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(NewPageTest, TestTag) {
   static const constexpr PageIndex kDefaultPageIndex = 0;

   static const constexpr RawPageTag kDefaultPageTag = PageTag::New(kDefaultPageIndex);
   MemoryRegion region(GetNewPageSize());
   NewPage page(kDefaultPageIndex, region);
   SetTag(page, kDefaultPageTag);
   ASSERT_EQ(page.tag(), PageTag(kDefaultPageTag));

   static const constexpr RawPageTag kUpdatedPageTag = PageTag::Old(kDefaultPageIndex);
   SetTag(page, kUpdatedPageTag);
   ASSERT_EQ(page.tag(), PageTag(kUpdatedPageTag));
 }

 TEST_F(NewPageTest, TestTryAllocate) {
   MemoryRegion region(GetNewPageSize());
   NewPage page(0, region);
   auto ptr = TryAllocateWord(page, 100);
   ASSERT_TRUE(IsAllocated(ptr));
   ASSERT_TRUE(IsNew(ptr));
   ASSERT_FALSE(IsOld(ptr));
   ASSERT_FALSE(IsMarked(ptr));
   ASSERT_FALSE(IsRemembered(ptr));
   ASSERT_FALSE(IsForwarding(ptr));
   ASSERT_TRUE(IsWord(ptr, 100));
   ASSERT_TRUE(page.marked());
   ASSERT_TRUE(page.Contains(*ptr));
 }

 TEST_F(NewPageTest, TestVisitPointers) {
   MemoryRegion region(GetNewPageSize());
   NewPage page(0, region);

   static const constexpr int64_t kNumberOfPointers = 10;

   for(auto idx = 0; idx < kNumberOfPointers; idx++) {
     auto ptr = TryAllocateWord(page, idx);
     ASSERT_TRUE(IsAllocated(ptr));
     ASSERT_TRUE(IsNew(ptr));
     ASSERT_FALSE(IsOld(ptr));
     ASSERT_FALSE(IsMarked(ptr));
     ASSERT_FALSE(IsRemembered(ptr));
     ASSERT_FALSE(IsForwarding(ptr));
     ASSERT_TRUE(IsWord(ptr, idx));
     ASSERT_TRUE(page.marked());
     ASSERT_TRUE(page.Contains(*ptr));
   }

   MockRawObjectVisitor visitor;
   EXPECT_CALL(visitor, Visit)
     .Times(kNumberOfPointers);
   ASSERT_NO_FATAL_FAILURE(page.VisitPointers(&visitor));
 }

 TEST_F(NewPageTest, TestVisitMarkedPages) {
   MemoryRegion region(GetNewPageSize());
   NewPage page(0, region);

   static const constexpr int64_t kNumberOfPointers = 10;
   static const constexpr int64_t kNumberOfMarkedPointers = 4;

   for(auto idx = 0; idx < kNumberOfPointers; idx++) {
     auto ptr = TryAllocateWord(page, idx);
     ASSERT_TRUE(IsAllocated(ptr));
     ASSERT_TRUE(IsNew(ptr));
     ASSERT_FALSE(IsOld(ptr));
     ASSERT_FALSE(IsMarked(ptr));
     ASSERT_FALSE(IsRemembered(ptr));
     ASSERT_FALSE(IsForwarding(ptr));
     ASSERT_TRUE(IsWord(ptr, idx));
     ASSERT_TRUE(page.marked());
     ASSERT_TRUE(page.Contains(*ptr));
   }

   for(auto idx = 0; idx < kNumberOfMarkedPointers; idx++) {
     auto ptr = TryAllocateMarkedWord(page, idx);
     ASSERT_TRUE(IsAllocated(ptr));
     ASSERT_TRUE(IsNew(ptr));
     ASSERT_FALSE(IsOld(ptr));
     ASSERT_TRUE(IsMarked(ptr));
     ASSERT_FALSE(IsRemembered(ptr));
     ASSERT_FALSE(IsForwarding(ptr));
     ASSERT_TRUE(IsWord(ptr, idx));
     ASSERT_TRUE(page.marked());
     ASSERT_TRUE(page.Contains(*ptr));
   }

   MockRawObjectVisitor visitor;
   EXPECT_CALL(visitor, Visit)
     .Times(kNumberOfMarkedPointers);
   ASSERT_NO_FATAL_FAILURE(page.VisitMarkedPointers(&visitor));
 }
}