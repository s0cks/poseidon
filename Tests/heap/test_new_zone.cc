#include "helpers.h"
#include "helpers/assertions.h"
#include "heap/test_new_zone.h"

namespace poseidon{
 TEST_F(NewZoneTest, TestTryAllocate){
   MemoryRegion region(GetNewZoneSize());
   NewZone zone(region);
   Semispace fromspace = GetFromspace(zone);

   static const constexpr word kDefaultWordValue = 42;
   auto ptr = TryAllocateNewWord(&zone, kDefaultWordValue);
   ASSERT_TRUE(IsAllocated(ptr));
   ASSERT_TRUE(IsNew(ptr));
   ASSERT_FALSE(IsOld(ptr));
   ASSERT_FALSE(IsMarked(ptr));
   ASSERT_FALSE(IsRemembered(ptr));
   ASSERT_FALSE(IsForwarding(ptr));
   ASSERT_TRUE(IsWord(ptr, kDefaultWordValue));
   ASSERT_TRUE(fromspace.Contains(ptr->GetStartingAddress()));

   // try and allocate an object thats in 2 pages
   auto p2 = (RawObject*)zone.TryAllocate(GetNewPageSize() * 2);
   *((word*)p2->GetPointer()) = kDefaultWordValue;
   ASSERT_TRUE(IsAllocated(p2));
   ASSERT_TRUE(IsNew(p2));
   ASSERT_FALSE(IsOld(p2));
   ASSERT_FALSE(IsMarked(p2));
   ASSERT_FALSE(IsRemembered(p2));
   ASSERT_FALSE(IsForwarding(p2));
   ASSERT_TRUE(fromspace.Contains(p2->GetStartingAddress()));
   LOG(INFO) << "new-zone marked: " << zone.marked_set();
 }

 TEST_F(NewZoneTest, TestVisitPointers) {
   MemoryRegion region(GetNewZoneSize());
   NewZone zone(region);
   static const constexpr int64_t kNumberOfPointers = 3;

   for(auto idx = 0; idx < kNumberOfPointers; idx++){
     auto ptr = TryAllocateNewWord(&zone, idx);
     ASSERT_TRUE(zone.Contains(ptr->GetStartingAddress()));
     ASSERT_TRUE(IsAllocated(ptr));
     ASSERT_TRUE(IsNew(ptr));
     ASSERT_TRUE(IsWord(ptr, idx));
     ASSERT_FALSE(IsMarked(ptr));
   }

   MockRawObjectVisitor visitor;
   EXPECT_CALL(visitor, Visit)
     .Times(kNumberOfPointers);
   ASSERT_NO_FATAL_FAILURE(zone.VisitPointers(&visitor));
 }

 TEST_F(NewZoneTest, TestVisitMarkedPointers) {
   MemoryRegion region(GetNewZoneSize());
   NewZone zone(region);
   static const constexpr int64_t kNumberOfUnmarkedPointers = 3;
   static const constexpr int64_t kNumberOfPointers = 3;

   for(auto idx = 0; idx < kNumberOfUnmarkedPointers; idx++){
     auto ptr = TryAllocateNewWord(&zone, idx);
     ASSERT_TRUE(zone.Contains(ptr->GetStartingAddress()));
     ASSERT_TRUE(IsAllocated(ptr));
     ASSERT_TRUE(IsNew(ptr));
     ASSERT_TRUE(IsWord(ptr, idx));
     ASSERT_FALSE(IsMarked(ptr));
   }

   for(auto idx = 0; idx < kNumberOfPointers; idx++){
     auto ptr = TryAllocateMarkedWord(&zone, idx);
     ASSERT_TRUE(zone.Contains(ptr->GetStartingAddress()));
     ASSERT_TRUE(IsAllocated(ptr));
     ASSERT_TRUE(IsNew(ptr));
     ASSERT_TRUE(IsWord(ptr, idx));
     ASSERT_TRUE(IsMarked(ptr));
   }

   MockRawObjectVisitor visitor;
   EXPECT_CALL(visitor, Visit)
     .Times(kNumberOfPointers);
   ASSERT_NO_FATAL_FAILURE(zone.VisitMarkedPointers(&visitor));
 }
}