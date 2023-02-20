#ifndef PSDN_TEST_NEW_PAGE_H
#define PSDN_TEST_NEW_PAGE_H

#include <gtest/gtest.h>
#include "poseidon/page/new_page.h"

namespace poseidon {
 class BaseMemoryRegionTest : public ::testing::Test {
  protected:
   MemoryRegion region_;

   explicit BaseMemoryRegionTest(const word size, const MemoryRegion::ProtectionMode mode = MemoryRegion::kNoAccess):
    ::testing::Test(),
    region_(size, mode) {
   }

   inline MemoryRegion& region() {
     return region_;
   }
  public:
   ~BaseMemoryRegionTest() override = default;

   void SetUp() override {

   }

   void TearDown() override {

   }
 };

 class NewPageTest : public BaseMemoryRegionTest {
  public:
   static constexpr const PageIndex kPageIndex = 0;
  protected:
   NewPage page_;

   NewPageTest():
    BaseMemoryRegionTest(flags::GetNewPageSize(), MemoryRegion::kReadOnly),
    page_(kPageIndex, region()) {
   }

   inline NewPage& page() {
     return page_;
   }
  public:
   ~NewPageTest() override = default;

   void SetUp() override {
     ASSERT_NO_FATAL_FAILURE(page().SetWritable());
     ASSERT_NO_FATAL_FAILURE(page().Clear());
     ASSERT_NO_FATAL_FAILURE(page().SetReadOnly());
     //TODO: print NewPage
     ASSERT_NO_FATAL_FAILURE(page().SetWritable());
   }

   void TearDown() override {
     ASSERT_NO_FATAL_FAILURE(page().SetReadOnly());
     //TODO: print NewPage
   }
 };
}

#endif //PSDN_TEST_NEW_PAGE_H