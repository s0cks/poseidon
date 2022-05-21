#include "helpers.h"
#include "heap/test_old_page.h"

namespace poseidon{
 TEST_F(OldPageTest, TestTryAllocate){
   static const constexpr word kDefaultWordValue = 42;
   auto ptr = TryAllocateNewWord(&page_, kDefaultWordValue);
   ASSERT_TRUE(IsAllocated(ptr));
   ASSERT_FALSE(IsNew(ptr));
   ASSERT_TRUE(IsOld(ptr));
   ASSERT_FALSE(IsMarked(ptr));
   ASSERT_FALSE(IsRemembered(ptr));
   ASSERT_FALSE(IsForwarding(ptr));
   ASSERT_EQ(ptr->GetPointerSize(), kWordSize);
 }

 TEST_F(OldPageTest, TestVisitPointers){
   static const constexpr int64_t kNumberOfPointers = 3;
   for(auto idx = 0; idx < kNumberOfPointers; idx++){
     auto ptr = TryAllocateNewWord(&page_, idx);
     ASSERT_TRUE(IsAllocated(ptr));
   }

   MockRawObjectVisitor visitor;
   EXPECT_CALL(visitor, Visit)
     .Times(kNumberOfPointers);
   ASSERT_NO_FATAL_FAILURE(page_.VisitPointers(&visitor));
 }

 TEST_F(OldPageTest, TestVisitMarkedPointers){
   static const constexpr int64_t kNumberOfUnmarkedPointers = 1;
   static const constexpr int64_t kNumberOfMarkedPointers = 3;

   for(auto idx = 0; idx < kNumberOfUnmarkedPointers; idx++){
     auto ptr = TryAllocateNewWord(&page_, idx);
     ASSERT_TRUE(IsAllocated(ptr));
     ASSERT_TRUE(page_.Contains(ptr->GetAddress()));
     ASSERT_TRUE(IsWord(ptr, idx));
     ASSERT_FALSE(IsMarked(ptr));
   }

   for(auto idx = 0; idx < kNumberOfMarkedPointers; idx++){
     auto ptr = TryAllocateNewMarkedWord(&page_, idx);
     ASSERT_TRUE(IsAllocated(ptr));
     ASSERT_TRUE(page_.Contains(ptr->GetAddress()));
     ASSERT_TRUE(IsWord(ptr, idx));
     ASSERT_TRUE(IsMarked(ptr));
   }

   MockRawObjectVisitor visitor;
   EXPECT_CALL(visitor, Visit)
     .Times(kNumberOfMarkedPointers);
   ASSERT_NO_FATAL_FAILURE(page_.VisitMarkedPointers(&visitor));
 }
}