#include "heap/test_page.h"

#include "poseidon/platform/memory_region.h"

namespace poseidon {
 TEST_F(PageTest, TestPageConstruction) {
   static const uint64_t kPageSize = 1024;
   MemoryRegion region(kPageSize);

   static const uint32_t kP1Index = 0;
   static const uword kP1Offset = kP1Index * kPageSize;

   auto p1 = new Page(kP1Index, region);
   ASSERT_EQ(p1->index(), kP1Index);
   ASSERT_FALSE(p1->marked());
   ASSERT_EQ(p1->GetStartingAddress(), region.GetStartingAddress() + kP1Offset);
   ASSERT_EQ(p1->GetCurrentAddress(), region.GetStartingAddress() + kP1Offset);
   ASSERT_EQ(p1->GetEndingAddress(), region.GetStartingAddress() + kP1Offset + kPageSize);
 }

 TEST_F(PageTest, TestTagMarkedBit) {
   static const uint64_t kPageSize = 1024;
   MemoryRegion region(kPageSize);

   static const uint32_t kP1Index = 0;
   static const uword kP1Offset = kP1Index * kPageSize;

   auto p1 = new Page(kP1Index, region);
   ASSERT_EQ(p1->index(), kP1Index);
   ASSERT_FALSE(p1->marked());
   ASSERT_EQ(p1->GetStartingAddress(), region.GetStartingAddress() + kP1Offset);
   ASSERT_EQ(p1->GetCurrentAddress(), region.GetStartingAddress() + kP1Offset);
   ASSERT_EQ(p1->GetEndingAddress(), region.GetStartingAddress() + kP1Offset + kPageSize);

   Mark(p1);
   ASSERT_TRUE(p1->marked());

   Unmark(p1);
   ASSERT_FALSE(p1->marked());
 }

 TEST_F(PageTest, TestTryAllocate) {
   NOT_IMPLEMENTED(ERROR); //TODO: implement
 }
}