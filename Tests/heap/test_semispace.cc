#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <glog/logging.h>

#include "poseidon/heap/zone_new.h"
#include "poseidon/heap/semispace.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon{
 using namespace ::testing;

 class SemispaceTest : public Test{
  public:
   static const constexpr int64_t kDefaultSemispaceSize = 2 * kMB;
  protected:
   MemoryRegion region_;
   Semispace semispace_;

   inline MemoryRegion* region(){
     return &region_;
   }

   void SetUp() override{
     ASSERT_TRUE(region()->Protect(MemoryRegion::kReadWrite)) << "cannot set " << region_ << " to " << MemoryRegion::kReadWrite;
   }
  public:
   SemispaceTest():
    region_(kDefaultSemispaceSize),
    semispace_(region()){
   }
   ~SemispaceTest() override = default;
 };

 TEST_F(SemispaceTest, TestTryAllocate){
   auto address = semispace_.TryAllocate(sizeof(uword));
   ASSERT_NE(address, 0);

   auto ptr = (RawObject*)address;
   ASSERT_FALSE(ptr->IsNew());
   ASSERT_FALSE(ptr->IsOld());
   ASSERT_FALSE(ptr->IsMarked());
   ASSERT_FALSE(ptr->IsRemembered());
   ASSERT_FALSE(ptr->IsForwarding());
   ASSERT_EQ(ptr->GetPointerSize(), sizeof(uword));
 }

 class MockSemispaceVisitor : public RawObjectVisitor{
  public:
   MockSemispaceVisitor():
    RawObjectVisitor(){
     ON_CALL(*this, Visit)
      .WillByDefault([](RawObject* val){
        return true;
      });
   }
   ~MockSemispaceVisitor() override = default;
   MOCK_METHOD(bool, Visit, (RawObject*), (override));
 };

 TEST_F(SemispaceTest, TestVisitPointers){
   auto p1 = (RawObject*)semispace_.TryAllocate(sizeof(uword));
   ASSERT_NE(p1, nullptr);
   auto p2 = (RawObject*)semispace_.TryAllocate(sizeof(uword));
   ASSERT_NE(p2, nullptr);
   auto p3 = (RawObject*)semispace_.TryAllocate(sizeof(uword));
   ASSERT_NE(p3, nullptr);

   MockSemispaceVisitor visitor;
   EXPECT_CALL(visitor, Visit)
    .Times(3);

   ASSERT_NO_FATAL_FAILURE(semispace_.VisitRawObjects(&visitor));
 }

 TEST_F(SemispaceTest, TestVisitMarkedPointers){
   auto p1 = (RawObject*)semispace_.TryAllocate(sizeof(uword));
   ASSERT_NE(p1, nullptr);

   auto p2 = (RawObject*)semispace_.TryAllocate(sizeof(uword));
   ASSERT_NE(p2, nullptr);
   p2->SetMarkedBit();

   auto p3 = (RawObject*)semispace_.TryAllocate(sizeof(uword));
   ASSERT_NE(p3, nullptr);
   p3->SetMarkedBit();

   MockSemispaceVisitor visitor;
   EXPECT_CALL(visitor, Visit)
    .Times(3);
   ASSERT_NO_FATAL_FAILURE(semispace_.VisitRawObjects(&visitor));

   EXPECT_CALL(visitor, Visit)
    .Times(2);
   ASSERT_NO_FATAL_FAILURE(semispace_.VisitMarkedObjects(&visitor));
 }
}