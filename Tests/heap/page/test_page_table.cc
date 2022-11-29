#include <gtest/gtest.h>

#include "poseidon/type/all.h"
#include "poseidon/heap/zone/new_zone.h"
#include "poseidon/heap/page/page_table.h"

#include "assertions/assertions.h"

namespace poseidon {
 using namespace ::testing;

 class PageTableTest : public Test {
  protected:
   PageTableTest() = default;
  public:
   ~PageTableTest() override = default;
 };

 TEST_F(PageTableTest, TestConstructor) {
   MemoryRegion region(flags::GetNewZoneSize());
   ASSERT_NO_FATAL_FAILURE(region.Protect(MemoryRegion::kReadWrite));
   NewZone zone(region);

   PageTable table(zone.GetStartingAddress(), zone.GetSize(), flags::GetNewPageSize());
   DLOG(INFO) << "table: " << table;
 }

 TEST_F(PageTableTest, TestIsMarked_WillPass) {
   MemoryRegion region(flags::GetNewZoneSize());
   ASSERT_NO_FATAL_FAILURE(region.Protect(MemoryRegion::kReadWrite));
   NewZone zone(region);
   PageTable table(zone.GetStartingAddress(), zone.GetSize(), flags::GetNewPageSize());
   ASSERT_FALSE(table.IsMarked(0));
   ASSERT_NO_FATAL_FAILURE(table.Mark(0));
   ASSERT_TRUE(table.IsMarked(0));
   ASSERT_NO_FATAL_FAILURE(table.Unmark(0));
   ASSERT_FALSE(table.IsMarked(0));
 }

 TEST_F(PageTableTest, TestMarkAllIntersectedBy_WillPass) {
   MemoryRegion region(flags::GetNewZoneSize());
   ASSERT_NO_FATAL_FAILURE(region.Protect(MemoryRegion::kReadWrite));
   NewZone zone(region);
   PageTable table(zone.GetStartingAddress(), zone.GetSize(), flags::GetNewPageSize());

   auto p1 = Long::TryAllocateIn(&zone, 10);
   ASSERT_NE(p1, nullptr);
   ASSERT_TRUE(IsLong(p1->raw_ptr()));
   ASSERT_TRUE(LongEq(10, p1));

   ASSERT_NO_FATAL_FAILURE(table.MarkAllIntersectedBy(p1->raw_ptr()));

   ASSERT_TRUE(table.IsMarked(0));
   DLOG(INFO) << "table: " << table;
 }
}