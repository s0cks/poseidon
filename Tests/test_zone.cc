#include <gtest/gtest.h>
#include <glog/logging.h>

#include "poseidon/zone.h"

namespace poseidon{
 using namespace ::testing;

 class ZoneTest : public Test{
  public:
   static const constexpr int64_t kDefaultRegionSize = 4 * kMB;
  protected:
   MemoryRegion region_;
   Zone zone_;
  public:
   ZoneTest():
    region_(kDefaultRegionSize),
    zone_(region_){
     if(!region_.Protect(MemoryRegion::kReadWrite)){
       LOG(FATAL) << "failed to protect " << region_;
       return;
     }
   }
   ~ZoneTest() override = default;
 };

 TEST_F(ZoneTest, TestTryAllocateWord){
   auto val = (RawObject*)zone_.TryAllocate(sizeof(word));
   ASSERT_EQ(val->GetPointerSize(), sizeof(word));
   ASSERT_TRUE(zone_.Contains(val->GetAddress()));
   ASSERT_TRUE(zone_.Contains(val->GetObjectPointerAddress()));
 }

 class NewZoneTest : public Test{
  public:
   static const constexpr int64_t kDefaultRegionSize = 4 * kMB;
  protected:
   MemoryRegion region_;
   NewZone zone_;
  public:
   NewZoneTest():
    region_(kDefaultRegionSize),
    zone_(region_){
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

 class OldZoneTest : public Test{
  protected:
   MemoryRegion region_;
   OldZone zone_;
  public:
   OldZoneTest():
    region_(GetOldZoneSize()),
    zone_(region_, 0, GetOldZoneSize(), GetOldPageSize()){
     if(!region_.Protect(MemoryRegion::kReadWrite)){
       LOG(FATAL) << "failed to protect " << region_;
       return;
     }
   }
   ~OldZoneTest() override = default;
 };

 TEST_F(OldZoneTest, TestPages){
   ASSERT_EQ(zone_.GetNumberOfPages(), 32);

   auto p1 = zone_.GetNumberOfPages();
 }

 TEST_F(OldZoneTest, TestTryAllocateWord){
   auto val = (RawObject*)zone_.TryAllocate(sizeof(word));
   ASSERT_EQ(val->GetPointerSize(), sizeof(word));
   ASSERT_TRUE(zone_.Contains(val->GetAddress()));
   ASSERT_TRUE(zone_.Contains(val->GetObjectPointerAddress()));
 }
}