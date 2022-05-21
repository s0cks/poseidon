#ifndef POSEIDON_TEST_NEW_ZONE_H
#define POSEIDON_TEST_NEW_ZONE_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "poseidon/flags.h"
#include "memory_region_test.h"
#include "poseidon/heap/new_zone.h"

namespace poseidon{
 using namespace ::testing;

 class NewZoneTest : public MemoryRegionTest{
  protected:
   NewZone zone_;
  public:
   NewZoneTest():
    MemoryRegionTest(GetNewZoneSize()),
    zone_(region()){
   }
   ~NewZoneTest() override = default;
 };
}

#endif//POSEIDON_TEST_NEW_ZONE_H