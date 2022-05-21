#ifndef POSEIDON_TEST_NEW_ZONE_H
#define POSEIDON_TEST_NEW_ZONE_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "poseidon/flags.h"
#include "memory_region_test.h"
#include "poseidon/heap/new_zone.h"
#include "poseidon/heap/semispace.h"

namespace poseidon{
 using namespace ::testing;

 class NewZoneTest : public MemoryRegionTest{
  protected:
   NewZone zone_;
   Semispace from_;
   Semispace to_;

   inline NewZone* zone(){
     return &zone_;
   }

   inline Semispace* from(){
     return &from_;
   }

   inline Semispace* to(){
     return &to_;
   }
  public:
   NewZoneTest():
    MemoryRegionTest(GetNewZoneSize()),
    zone_(region()),
    from_(zone()->fromspace(), zone()->semisize()),
    to_(zone()->tospace(), zone()->semisize()){
   }
   ~NewZoneTest() override = default;
 };
}

#endif//POSEIDON_TEST_NEW_ZONE_H