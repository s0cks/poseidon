#include <gtest/gtest.h>

#include "poseidon/zone.h"

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

   auto new_ptr = zone.Allocate(sizeof(uword));
   ASSERT_NE(new_ptr, 0);

   DLOG(INFO) << "objects: ";
   zone.VisitMarkedPages([&](OldPage* page){
     page->VisitPointers([&](RawObject* val){
       DLOG(INFO) << " - " << val->ToString();
       return true;
     });
     return true;
   });
 }
}