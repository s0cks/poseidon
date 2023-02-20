#ifndef PSDN_TEST_NEW_PAGE_H
#define PSDN_TEST_NEW_PAGE_H

#include <gtest/gtest.h>
#include "poseidon/page/new_page.h"
#include "base_memory_region_test.h"

namespace poseidon {
 class NewPageTest : public BaseMemoryRegionTest {
  public:
   static constexpr const PageIndex kTestPageIndex = 0;
   static constexpr const Size kTestPageSize = Megabytes(2);
  protected:
   NewPage page_;

   NewPageTest():
    BaseMemoryRegionTest((RegionSize) kTestPageSize, MemoryRegion::kReadOnly),
    page_(kTestPageIndex, region()) {
   }

   inline NewPage& page() {
     return page_;
   }
  public:
   ~NewPageTest() override = default;

   void SetUp() override {
     BaseMemoryRegionTest::SetUp();
     ASSERT_NO_FATAL_FAILURE(page().SetWritable());
     ASSERT_NO_FATAL_FAILURE(page().Clear());
     ASSERT_NO_FATAL_FAILURE(page().SetReadOnly());
     //TODO: print NewPage
     ASSERT_NO_FATAL_FAILURE(page().SetWritable());
   }

   void TearDown() override {
     BaseMemoryRegionTest::TearDown();
     ASSERT_NO_FATAL_FAILURE(page().SetReadOnly());
     //TODO: print NewPage
   }
 };
}

#endif //PSDN_TEST_NEW_PAGE_H