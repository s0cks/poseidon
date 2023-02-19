#include <gtest/gtest.h>

#include "poseidon/object.h"
#include "poseidon/zone/new_zone.h"
#include "poseidon/heap/page/page_table.h"

#include "assertions/assertions.h"

namespace poseidon {
 using namespace ::testing;

 class PageTableTest : public Test {
  protected:
   MemoryRegion test_region_;
   PageTable table_;

   PageTableTest():
    test_region_(flags::GetNewZoneSize()),
    table_((const Region&) test_region_, flags::GetNewPageSize()) {
   }

   MemoryRegion& region() {
     return test_region_;
   }

   PageTable& table() {
     return table_;
   }
  public:
   ~PageTableTest() override = default;

   void SetUp() override {
     ASSERT_TRUE(region().Protect(MemoryRegion::kReadOnly));
     ASSERT_NO_FATAL_FAILURE(PageTablePrinter<>::Print(table(), 50, "Before"));
     for(auto idx = 0; idx < table().GetNumberOfPages(); idx++)
       ASSERT_FALSE(table().IsMarked(idx));
     ASSERT_TRUE(region().Protect(MemoryRegion::kReadWrite));
     ASSERT_NO_FATAL_FAILURE(region().ClearRegion());
     ASSERT_NO_FATAL_FAILURE(table().Clear());
   }

   void TearDown() override {
     ASSERT_TRUE(region().Protect(MemoryRegion::kReadOnly));
     ASSERT_NO_FATAL_FAILURE(PageTablePrinter<>::Print(table(), 50, "After"));
   }
 };

 TEST_F(PageTableTest, TestIsMarked_WillPass) {
   NewZone zone(region());
   ASSERT_FALSE(table().IsMarked(0));
   ASSERT_NO_FATAL_FAILURE(table().Mark(0));
   ASSERT_TRUE(table().IsMarked(0));
   ASSERT_NO_FATAL_FAILURE(table().Unmark(0));
   ASSERT_FALSE(table().IsMarked(0));
 }

 TEST_F(PageTableTest, TestMarkAllIntersectedBy_WillPass) {
   NewZone zone(region());
   auto p1 = Int32::TryAllocateIn(&zone, 10);
   ASSERT_NE(p1, nullptr);
   ASSERT_TRUE(IsInt32(p1->raw_ptr()));
   ASSERT_TRUE(Int32Eq(10, p1));

   ASSERT_NO_FATAL_FAILURE(table().MarkAllIntersectedBy((Region) *p1->raw_ptr()));
   ASSERT_TRUE(table().IsMarked(0));
 }
}