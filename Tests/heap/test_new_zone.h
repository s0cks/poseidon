#ifndef POSEIDON_TEST_NEW_ZONE_H
#define POSEIDON_TEST_NEW_ZONE_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "poseidon/flags.h"
#include "poseidon/heap/new_zone.h"

namespace poseidon{
 using namespace ::testing;

 class NewZoneTest : public Test{
  protected:
   MemoryRegion region_;
   NewZone zone_;

   inline MemoryRegion* region(){
     return &region_;
   }

   void SetUp() override{
     ASSERT_TRUE(region()->Protect(MemoryRegion::kReadWrite)) << "cannot set " << region_ << " to " << MemoryRegion::kReadWrite;
   }
  public:
   NewZoneTest():
    region_(GetNewZoneSize()),
    zone_(region()){
   }
   ~NewZoneTest() override = default;
 };
}

#endif//POSEIDON_TEST_NEW_ZONE_H