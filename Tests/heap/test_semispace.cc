#include "helpers.h"
#include "helpers/assertions.h"
#include "heap/test_semispace.h"

namespace poseidon{
 using namespace ::testing;

 TEST_F(SemispaceTest, TestTryAllocate){
   static const constexpr word kDefaultWordValue = 42;
   auto ptr = TryAllocateNewWord(&semispace_, kDefaultWordValue);
   ASSERT_TRUE(IsAllocated(ptr));
   ASSERT_TRUE(IsNew(ptr));
   ASSERT_FALSE(IsOld(ptr));
   ASSERT_FALSE(IsMarked(ptr));
   ASSERT_FALSE(IsRemembered(ptr));
   ASSERT_FALSE(IsForwarding(ptr));
   ASSERT_EQ(ptr->GetPointerSize(), kWordSize);
 }

 TEST_F(SemispaceTest, TestVisitPointers){
   static const constexpr int64_t kNumberOfPointers = 3;
   for(auto idx = 0; idx < kNumberOfPointers; idx++){
     auto ptr = TryAllocateNewWord(&semispace_, idx);
     ASSERT_TRUE(IsAllocated(ptr));
   }

   MockRawObjectVisitor visitor;
   EXPECT_CALL(visitor, Visit)
    .Times(kNumberOfPointers);
   ASSERT_NO_FATAL_FAILURE(semispace_.VisitPointers(&visitor));
 }

 TEST_F(SemispaceTest, TestVisitMarkedPointers){
   static const constexpr int64_t kNumberOfUnmarkedPointers = 1;
   static const constexpr int64_t kNumberOfMarkedPointers = 3;

   for(auto idx = 0; idx < kNumberOfUnmarkedPointers; idx++){
     auto ptr = TryAllocateNewWord(&semispace_, idx);
     ASSERT_TRUE(IsAllocated(ptr));
     ASSERT_TRUE(semispace_.Contains(ptr->GetAddress()));
     ASSERT_TRUE(IsWord(ptr, idx));
     ASSERT_FALSE(IsMarked(ptr));
   }

   for(auto idx = 0; idx < kNumberOfMarkedPointers; idx++){
     auto ptr = TryAllocateMarkedWord(&semispace_, idx);
     ASSERT_TRUE(IsAllocated(ptr));
     ASSERT_TRUE(semispace_.Contains(ptr->GetAddress()));
     ASSERT_TRUE(IsWord(ptr, idx));
     ASSERT_TRUE(IsMarked(ptr));
   }

   MockRawObjectVisitor visitor;
   EXPECT_CALL(visitor, Visit)
    .Times(kNumberOfMarkedPointers);
   ASSERT_NO_FATAL_FAILURE(semispace_.VisitMarkedObjects(&visitor));
 }
}