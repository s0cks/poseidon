#include <gtest/gtest.h>

#include "poseidon/zone.h"
#include "poseidon/heap.h"

namespace poseidon{
 using namespace ::testing;

 class NewZoneTest : public Test{
  public:
   NewZoneTest() = default;
   ~NewZoneTest() override = default;
 };

 TEST_F(NewZoneTest, TestAllocate){

 }

 class OldZoneTest : public Test{
  public:
   OldZoneTest() = default;
   ~OldZoneTest() override = default;
 };

 TEST_F(OldZoneTest, TestAllocate){
   MemoryRegion region(kDefaultOldZoneSize);
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));

   OldZone zone(&region);

   auto val = zone.Allocate(sizeof(uword));
 }
}