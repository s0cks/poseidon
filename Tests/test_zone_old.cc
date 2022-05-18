#include <gtest/gtest.h>
#include <glog/logging.h>

#include "poseidon/zone_old.h"

namespace poseidon{
 using namespace ::testing;

 static const int64_t kDefaultNumberOfPages = GetOldZoneSize() / GetOldPageSize();

 class OldZoneTest : public Test{
  protected:
   MemoryRegion region_;
   OldZone zone_;

   inline OldPage*
   GetPage(int64_t idx){
     return zone_.pages(idx);
   }
  public:
   OldZoneTest():
     region_(GetOldZoneSize()),
     zone_(&region_, 0, GetOldZoneSize(), GetOldPageSize()){
     DLOG(INFO) << "using " << region_;
     if(!region_.Protect(MemoryRegion::kReadWrite)){
       LOG(FATAL) << "failed to protect " << region_;
       return;
     }
   }
   ~OldZoneTest() override = default;
 };

 TEST_F(OldZoneTest, TestPages){
   static const int64_t kExpectedPageSize = GetOldPageSize();
   ASSERT_EQ(zone_.GetNumberOfPages(), kDefaultNumberOfPages);

   // check page dimensions
   for(auto idx = 0; idx < kDefaultNumberOfPages; idx++){
     auto page = GetPage(idx);
     DLOG(INFO) << "creating " << (*page);
     ASSERT_EQ(page->index(), idx);
     ASSERT_EQ(page->GetSize(), kExpectedPageSize);
   }
 }

 TEST_F(OldZoneTest, TestTryAllocateWord){
   auto val = (RawObject*)zone_.TryAllocate(sizeof(word));
   ASSERT_EQ(val->GetPointerSize(), sizeof(word));
   ASSERT_TRUE(zone_.Contains(val->GetAddress()));
   ASSERT_TRUE(zone_.Contains(val->GetObjectPointerAddress()));
 }
}