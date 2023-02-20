#ifndef PSDN_BASE_MEMORY_REGION_TEST_H
#define PSDN_BASE_MEMORY_REGION_TEST_H

#include <gtest/gtest.h>
#include "poseidon/platform/memory_region.h"

namespace poseidon {
 class BaseMemoryRegionTest : public ::testing::Test {
  protected:
   MemoryRegion region_;

   explicit BaseMemoryRegionTest(const word size,
                                 const MemoryRegion::ProtectionMode mode = MemoryRegion::kNoAccess):
     ::testing::Test(),
     region_(size, mode) {
   }

   inline MemoryRegion& region() {
     return region_;
   }
  public:
   ~BaseMemoryRegionTest() override = default;

   void SetUp() override {
     ASSERT_NO_FATAL_FAILURE(region().Protect(MemoryRegion::kReadWrite));
     ASSERT_NO_FATAL_FAILURE(region().ClearRegion());
   }

   void TearDown() override {
     ASSERT_NO_FATAL_FAILURE(region().Protect(MemoryRegion::kReadOnly));
   }
 };
}

#endif //PSDN_BASE_MEMORY_REGION_TEST_H