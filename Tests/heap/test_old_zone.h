#ifndef POSEIDON_TEST_OLD_ZONE_H
#define POSEIDON_TEST_OLD_ZONE_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "memory_region_test.h"
#include "poseidon/heap/old_zone.h"

namespace poseidon{
 using namespace ::testing;

 class OldZoneTest : public MemoryRegionTest{
  protected:
   OldZone zone_;

   inline OldZone* zone(){
     return &zone_;
   }
  public:
   explicit OldZoneTest(int64_t size = GetOldZoneSize()):
     MemoryRegionTest(size),
     zone_(region(), GetOldPageSize()){
   }
   ~OldZoneTest() override = default;
 };
}

#endif//POSEIDON_TEST_OLD_ZONE_H