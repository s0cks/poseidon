#include "helpers.h"
#include "helpers/assertions.h"
#include "heap/test_semispace.h"

namespace poseidon{
 using namespace ::testing;

 TEST_F(SemispaceTest, TestConstruction) {

 }

 TEST_F(SemispaceTest, TestTryAllocate){
   MemoryRegion region(GetNewZoneSize() / 2); //TODO: cleanup
   Semispace semispace(region);

   static const constexpr word kDefaultWordValue = 42;
   auto ptr = TryAllocateNewWord(&semispace, kDefaultWordValue);
   ASSERT_TRUE(IsAllocated(ptr));
   ASSERT_TRUE(IsNew(ptr));
   ASSERT_FALSE(IsOld(ptr));
   ASSERT_FALSE(IsMarked(ptr));
   ASSERT_FALSE(IsRemembered(ptr));
   ASSERT_FALSE(IsForwarding(ptr));
   ASSERT_EQ(ptr->GetPointerSize(), kWordSize);
 }

 TEST_F(SemispaceTest, TestVisitPointers){
   MemoryRegion region(GetNewZoneSize() / 2); //TODO: cleanup
   Semispace semispace(region);

   static const constexpr int64_t kNumberOfPointers = 3;
   for(auto idx = 0; idx < kNumberOfPointers; idx++){
     auto ptr = TryAllocateNewWord(&semispace, idx);
     ASSERT_TRUE(IsAllocated(ptr));
   }

   MockRawObjectVisitor visitor;
   EXPECT_CALL(visitor, Visit)
    .Times(kNumberOfPointers);
   ASSERT_NO_FATAL_FAILURE(semispace.VisitPointers(&visitor));
 }

 TEST_F(SemispaceTest, TestVisitMarkedPointers){
   MemoryRegion region(GetNewZoneSize() / 2); //TODO: cleanup
   Semispace semispace(region);

   static const constexpr int64_t kNumberOfUnmarkedPointers = 1;
   static const constexpr int64_t kNumberOfMarkedPointers = 3;

   for(auto idx = 0; idx < kNumberOfUnmarkedPointers; idx++){
     auto ptr = TryAllocateNewWord(&semispace, idx);
     ASSERT_TRUE(IsAllocated(ptr));
     ASSERT_TRUE(semispace.Contains(ptr->GetStartingAddress()));
     ASSERT_TRUE(IsWord(ptr, idx));
     ASSERT_FALSE(IsMarked(ptr));
   }

   for(auto idx = 0; idx < kNumberOfMarkedPointers; idx++){
     auto ptr = TryAllocateMarkedWord(&semispace, idx);
     ASSERT_TRUE(IsAllocated(ptr));
     ASSERT_TRUE(semispace.Contains(ptr->GetStartingAddress()));
     ASSERT_TRUE(IsWord(ptr, idx));
     ASSERT_TRUE(IsMarked(ptr));
   }

   MockRawObjectVisitor visitor;
   EXPECT_CALL(visitor, Visit)
    .Times(kNumberOfMarkedPointers);
   ASSERT_NO_FATAL_FAILURE(semispace.VisitMarkedObjects(&visitor));
 }
}