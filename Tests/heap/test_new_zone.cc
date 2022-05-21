#include "helpers.h"
#include "heap/test_new_zone.h"

namespace poseidon{
 TEST_F(NewZoneTest, TestTryAllocate){
   static const constexpr word kDefaultWordValue = 42;
   auto ptr = TryAllocateNewWord(zone(), kDefaultWordValue);
   ASSERT_TRUE(IsAllocated(ptr));
   ASSERT_TRUE(IsNew(ptr));
   ASSERT_FALSE(IsOld(ptr));
   ASSERT_FALSE(IsMarked(ptr));
   ASSERT_FALSE(IsRemembered(ptr));
   ASSERT_FALSE(IsForwarding(ptr));
   ASSERT_TRUE(IsWord(ptr, kDefaultWordValue));
   ASSERT_TRUE(from()->Contains(ptr->GetAddress()));
 }

 TEST_F(NewZoneTest, TestVisitPointers){
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

 TEST_F(NewZoneTest, TestVisitMarkedPointers){
   static const constexpr int64_t kNumberOfUnmarkedPointers = 1;
   static const constexpr int64_t kNumberOfMarkedPointers = 3;

   for(auto idx = 0; idx < kNumberOfUnmarkedPointers; idx++){
     auto ptr = TryAllocateNewWord(zone(), idx);
     ASSERT_TRUE(IsAllocated(ptr));
     ASSERT_TRUE(zone()->Contains(ptr->GetAddress()));
     ASSERT_TRUE(IsWord(ptr, idx));
     ASSERT_FALSE(IsMarked(ptr));
     ASSERT_TRUE(from()->Contains(ptr->GetAddress()));
   }

   for(auto idx = 0; idx < kNumberOfMarkedPointers; idx++){
     auto ptr = TryAllocateNewMarkedWord(zone(), idx);
     ASSERT_TRUE(IsAllocated(ptr));
     ASSERT_TRUE(zone()->Contains(ptr->GetAddress()));
     ASSERT_TRUE(IsWord(ptr, idx));
     ASSERT_TRUE(IsMarked(ptr));
     ASSERT_TRUE(from()->Contains(ptr->GetAddress()));
   }

   MockRawObjectVisitor visitor;
   EXPECT_CALL(visitor, Visit)
     .Times(kNumberOfMarkedPointers);
   ASSERT_NO_FATAL_FAILURE(zone()->VisitMarkedPointers(&visitor));
 }
}