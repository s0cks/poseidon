#include <gtest/gtest.h>
#include <glog/logging.h>

#include "poseidon/zone_new.h"

namespace poseidon{
 using namespace ::testing;

 class NewZoneTest : public Test{
  public:
   static const constexpr int64_t kDefaultRegionSize = 4 * kMB;
  protected:
   MemoryRegion region_;
   NewZone zone_;
  public:
   NewZoneTest():
     region_(kDefaultRegionSize),
     zone_(&region_){
     if(!region_.Protect(MemoryRegion::kReadWrite)){
       LOG(FATAL) << "failed to protect " << region_;
       return;
     }
   }
   ~NewZoneTest() override = default;
 };

 TEST_F(NewZoneTest, TestTryAllocateWord){
   auto val = (RawObject*)zone_.TryAllocate(sizeof(word));
   ASSERT_EQ(val->GetPointerSize(), sizeof(word));
   ASSERT_TRUE(zone_.Contains(val->GetAddress()));
   ASSERT_TRUE(zone_.Contains(val->GetObjectPointerAddress()));

   Semispace from(zone_.fromspace(), zone_.semisize());
   ASSERT_TRUE(from.Contains(val->GetAddress()));
   ASSERT_TRUE(from.Contains(val->GetObjectPointerAddress()));
 }
}