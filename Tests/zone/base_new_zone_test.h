#ifndef PSDN_BASE_NEW_ZONE_TEST_H
#define PSDN_BASE_NEW_ZONE_TEST_H

#include "poseidon/zone/new_zone.h"
#include "base_memory_region_test.h"

namespace poseidon {
 class BaseNewZoneTest : public BaseMemoryRegionTest {
  protected:
   NewZone zone_;

   explicit BaseNewZoneTest(const RegionSize size = flags::GetNewZoneSize(),
                            const MemoryRegion::ProtectionMode mode = MemoryRegion::kNoAccess):
    BaseMemoryRegionTest(size, mode),
    zone_(region()) {
   }

   inline NewZone& zone() {
     return zone_;
   }
  public:
   ~BaseNewZoneTest() override = default;

   void SetUp() override {
     BaseMemoryRegionTest::SetUp();
     ASSERT_NO_FATAL_FAILURE(zone().SetReadOnly());
     ASSERT_TRUE(NewZonePrinter::Print(&zone()));
     ASSERT_NO_FATAL_FAILURE(zone().SetWritable());
     ASSERT_NO_FATAL_FAILURE(zone().Clear());
   }

   void TearDown() override {
     BaseMemoryRegionTest::TearDown();
     ASSERT_NO_FATAL_FAILURE(zone().SetReadOnly());
     ASSERT_TRUE(NewZonePrinter::Print(&zone()));
   }
 };
}

#endif //PSDN_BASE_NEW_ZONE_TEST_H