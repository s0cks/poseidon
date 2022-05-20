#include <gtest/gtest.h>
#include <glog/logging.h>

#include "poseidon/heap/zone.h"

namespace poseidon{
 using namespace ::testing;

 class ZoneTest : public Test{
  public:
   static const constexpr int64_t kDefaultRegionSize = 4 * kMB;
  protected:
   MemoryRegion region_;
   Zone zone_;

   inline MemoryRegion* region(){
     return &region_;
   }

   void SetUp() override{
     ASSERT_TRUE(region()->Protect(MemoryRegion::kReadWrite)) << "cannot set " << region_ << " to " << MemoryRegion::kReadWrite;
   }
  public:
   ZoneTest():
    region_(kDefaultRegionSize),
    zone_(&region_){
   }
   ~ZoneTest() override = default;
 };

 TEST_F(ZoneTest, TestTryAllocateWord){
   auto val = (RawObject*)zone_.TryAllocate(sizeof(word));
   ASSERT_EQ(val->GetPointerSize(), sizeof(word));
   ASSERT_TRUE(zone_.Contains(val->GetAddress()));
   ASSERT_TRUE(zone_.Contains(val->GetObjectPointerAddress()));
 }
}