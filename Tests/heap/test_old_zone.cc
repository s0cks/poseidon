#include "helpers.h"
#include "heap/test_old_zone.h"

namespace poseidon{
 TEST_F(OldZoneTest, TestTryAllocate){
   static const constexpr word kDefaultWordValue = 42;
   auto ptr = TryAllocateNewWord(zone(), kDefaultWordValue);
   ASSERT_TRUE(IsAllocated(ptr));
   ASSERT_FALSE(IsNew(ptr));
   ASSERT_TRUE(IsOld(ptr));
   ASSERT_FALSE(IsMarked(ptr));
   ASSERT_FALSE(IsRemembered(ptr));
   ASSERT_FALSE(IsForwarding(ptr));
   ASSERT_TRUE(IsWord(ptr, kDefaultWordValue));
 }

 TEST_F(OldZoneTest, TestPageTable){
   static constexpr const int64_t kDefaultValue = 42;
   auto p1 = TryAllocateNewWord(zone(), kDefaultValue);
   ASSERT_TRUE(IsAllocated(p1));
   ASSERT_TRUE(IsOld(p1));
   ASSERT_TRUE(IsWord(p1, kDefaultValue));
   ASSERT_FALSE(IsMarked(p1));
   ASSERT_FALSE(IsRemembered(p1));
   ASSERT_FALSE(IsForwarding(p1));

   auto page = zone()->pages(5);
   auto p2 = TryAllocateNewWord(page, kDefaultValue);
   ASSERT_TRUE(IsAllocated(p2));
   ASSERT_TRUE(IsOld(p2));
   ASSERT_TRUE(IsWord(p2, kDefaultValue));
   ASSERT_FALSE(IsMarked(p2));
   ASSERT_FALSE(IsRemembered(p2));
   ASSERT_FALSE(IsForwarding(p2));

   auto table = zone()->marked();
   DLOG(INFO) << "table: " << table;
 }

 TEST_F(OldZoneTest, TestVisitPointers){
   static const constexpr int64_t kNumberOfPointers = 3;
   for(auto idx = 0; idx < kNumberOfPointers; idx++){
     auto ptr = TryAllocateNewWord(zone(), idx);
     ASSERT_TRUE(IsAllocated(ptr));
   }

   MockRawObjectVisitor visitor;
   EXPECT_CALL(visitor, Visit)
     .Times(kNumberOfPointers);
   ASSERT_NO_FATAL_FAILURE(zone()->VisitPointers(&visitor));
 }

 TEST_F(OldZoneTest, TestVisitMarkedPointers){
   static const constexpr int64_t kNumberOfUnmarkedPointers = 1;
   static const constexpr int64_t kNumberOfMarkedPointers = 3;

   for(auto idx = 0; idx < kNumberOfUnmarkedPointers; idx++){
     auto ptr = TryAllocateNewWord(zone(), idx);
     ASSERT_TRUE(IsAllocated(ptr));
     ASSERT_TRUE(zone()->Contains(ptr->GetAddress()));
     ASSERT_TRUE(IsWord(ptr, idx));
     ASSERT_FALSE(IsMarked(ptr));
   }

   for(auto idx = 0; idx < kNumberOfMarkedPointers; idx++){
     auto ptr = TryAllocateMarkedWord(zone(), idx);
     ASSERT_TRUE(IsAllocated(ptr));
     ASSERT_TRUE(zone()->Contains(ptr->GetAddress()));
     ASSERT_TRUE(IsWord(ptr, idx));
     ASSERT_TRUE(IsMarked(ptr));
   }

   MockRawObjectVisitor visitor;
   EXPECT_CALL(visitor, Visit)
     .Times(kNumberOfMarkedPointers);
   ASSERT_NO_FATAL_FAILURE(zone()->VisitMarkedPointers(&visitor));
 }
}