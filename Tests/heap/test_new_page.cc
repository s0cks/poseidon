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
  public:
   ~NewPageTest() override = default;
 };

 TEST_F(NewPageTest, TestTryAllocateWillFailEqualToZero) {
   MemoryRegion region(GetNewPageSize());
   NewPage page(0, region);
   auto ptr = page.TryAllocate(0);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(NewPageTest, TestTryAllocateWillFailLessThanZero) {
   MemoryRegion region(GetNewPageSize());
   NewPage page(0, region);
   auto ptr = page.TryAllocate(-1);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(NewPageTest, TestTryAllocateWillFailEqualToPageSize) {
   MemoryRegion region(GetNewPageSize());
   NewPage page(0, region);
   auto ptr = page.TryAllocate(GetNewPageSize());
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(NewPageTest, TestTryAllocateWillFailGreaterThanPageSize) {
   MemoryRegion region(GetNewPageSize());
   NewPage page(0, region);
   auto ptr = page.TryAllocate(GetNewPageSize() + 1);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(NewPageTest, TestTryAllocate) {
   MemoryRegion region(GetNewPageSize());
   NewPage page(0, region);
   auto ptr = TryAllocateNewWord(&page, 100);
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
     auto ptr = TryAllocateNewWord(&page, idx);
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
     auto ptr = TryAllocateNewWord(&page, idx);
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
     auto ptr = TryAllocateMarkedWord(&page, idx);
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