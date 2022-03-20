#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <glog/logging.h>

#include "helpers.h"
#include "mock_zone.h"
#include "poseidon/heap.h"

namespace poseidon{
 using namespace ::testing;

 class HeapTest : public Test{
  protected:
   MockMemoryRegion region_;
   MockNewZone new_zone_;
   MockOldZone old_zone_;
   Heap* heap_;

   inline MockMemoryRegion& region(){
     return region_;
   }

   inline MockNewZone& new_zone(){
     return new_zone_;
   }

   inline MockOldZone& old_zone(){
     return old_zone_;
   }

   inline Heap* heap() const{
     return heap_;
   }

   void SetUp() override{
     delete heap_;
     EXPECT_CALL(region_, Protect(_))
       .WillRepeatedly(::testing::Return(true));
     heap_ = new Heap(&region_, &new_zone_, &old_zone_);
   }
  public:
   HeapTest():
    region_(),
    new_zone_(),
    old_zone_(),
    heap_(nullptr){
   }
   ~HeapTest() override = default;
 };

 TEST_F(HeapTest, TestAllocateNewObjectSuccessful){
   EXPECT_CALL(new_zone_, Allocate(_))
    .WillRepeatedly(AllocateNewObjectUsingSystem);

   auto val = (RawObject*)heap()->Allocate(sizeof(word));

   ASSERT_EQ(val->GetPointerSize(), kWordSize);
   ASSERT_TRUE(val->IsNew());
   ASSERT_FALSE(val->IsOld());
   ASSERT_FALSE(val->IsMarked());
   ASSERT_FALSE(val->IsRemembered());

   free(val);
 }

 TEST_F(HeapTest, TestAllocateNewObjectSuccessfulWithRetry){
   EXPECT_CALL(new_zone_, Allocate(_))
     .WillOnce(FailAllocation)
     .WillRepeatedly(AllocateNewObjectUsingSystem);

   auto val = (RawObject*)heap()->Allocate(sizeof(word));

   ASSERT_EQ(val->GetPointerSize(), kWordSize);
   ASSERT_TRUE(val->IsNew());
   ASSERT_FALSE(val->IsOld());
   ASSERT_FALSE(val->IsMarked());
   ASSERT_FALSE(val->IsRemembered());

   free(val);
 }
}