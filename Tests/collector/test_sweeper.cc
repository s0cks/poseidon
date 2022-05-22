#include "helpers/assertions.h"
#include "collector/test_sweeper.h"

namespace poseidon{
 TEST_F(SweeperTest, TestSweepObject){
   static constexpr const int64_t kPtr1Value = 10;
   auto p1 = TryAllocateNewMarkedWord(zone(), kPtr1Value);
   ASSERT_TRUE(IsAllocated(p1));
   ASSERT_FALSE(IsNew(p1));
   ASSERT_TRUE(IsOld(p1));
   ASSERT_TRUE(IsMarked(p1));
   ASSERT_FALSE(IsRemembered(p1));
   ASSERT_FALSE(IsForwarding(p1));
   ASSERT_TRUE(IsWord(p1, kPtr1Value));

   static constexpr const int64_t kPtr2Value = 10000;
   auto p2 = TryAllocateNewWord(zone(), kPtr2Value);
   ASSERT_TRUE(IsAllocated(p2));
   ASSERT_FALSE(IsNew(p2));
   ASSERT_TRUE(IsOld(p2));
   ASSERT_FALSE(IsMarked(p2));
   ASSERT_FALSE(IsRemembered(p2));
   ASSERT_FALSE(IsForwarding(p2));
   ASSERT_TRUE(IsWord(p2, kPtr2Value));

   // should do nothing to p1 since it is marked.
   ASSERT_NO_FATAL_FAILURE(SweepObject(p1));
   ASSERT_TRUE(IsAllocated(p1));
   ASSERT_FALSE(IsNew(p1));
   ASSERT_TRUE(IsOld(p1));
   ASSERT_TRUE(IsMarked(p1));
   ASSERT_FALSE(IsRemembered(p1));
   ASSERT_FALSE(IsForwarding(p1));
   ASSERT_TRUE(IsWord(p1, kPtr1Value));

   // should finalize p2 since it is not marked.
   ASSERT_NO_FATAL_FAILURE(SweepObject(p2));
   ASSERT_TRUE(IsAllocated(p2));
   ASSERT_FALSE(IsNew(p2));
   ASSERT_TRUE(IsOld(p2));
   ASSERT_FALSE(IsMarked(p2));
   ASSERT_FALSE(IsRemembered(p2));
   ASSERT_FALSE(IsForwarding(p2));
   ASSERT_EQ(p2->GetPointerSize(), 0);
 }
 
 TEST_F(SweeperTest, TestSerialSweep){
   static constexpr const int64_t kPtr1Value = 10;
   auto p1 = TryAllocateNewMarkedWord(zone(), kPtr1Value);
   ASSERT_TRUE(IsAllocated(p1));
   ASSERT_FALSE(IsNew(p1));
   ASSERT_TRUE(IsOld(p1));
   ASSERT_TRUE(IsMarked(p1));
   ASSERT_FALSE(IsRemembered(p1));
   ASSERT_FALSE(IsForwarding(p1));
   ASSERT_TRUE(IsWord(p1, kPtr1Value));

   static constexpr const int64_t kPtr2Value = 100;
   auto p2 = TryAllocateNewMarkedWord(zone(), kPtr2Value);
   ASSERT_TRUE(IsAllocated(p2));
   ASSERT_FALSE(IsNew(p2));
   ASSERT_TRUE(IsOld(p2));
   ASSERT_TRUE(IsMarked(p2));
   ASSERT_FALSE(IsRemembered(p2));
   ASSERT_FALSE(IsForwarding(p2));
   ASSERT_TRUE(IsWord(p2, kPtr2Value));
   
   static constexpr const int64_t kGarbage1Value = 11111;
   auto g1 = TryAllocateNewWord(zone(), kGarbage1Value);
   ASSERT_TRUE(IsAllocated(g1));
   ASSERT_FALSE(IsNew(g1));
   ASSERT_TRUE(IsOld(g1));
   ASSERT_FALSE(IsMarked(g1));
   ASSERT_FALSE(IsRemembered(g1));
   ASSERT_FALSE(IsForwarding(g1));
   ASSERT_TRUE(IsWord(g1, kGarbage1Value));

   static constexpr const int64_t kGarbage2Value = 22222;
   auto g2 = TryAllocateNewWord(zone(), kGarbage2Value);
   // g2 should be:
   ASSERT_TRUE(IsAllocated(g2));
   ASSERT_TRUE(IsOld(g2));
   ASSERT_TRUE(IsWord(g2, kGarbage2Value));
   // g2 shouldn't be:
   ASSERT_FALSE(IsNew(g2));
   ASSERT_FALSE(IsMarked(g2));
   ASSERT_FALSE(IsRemembered(g2));
   ASSERT_FALSE(IsForwarding(g2));

   DLOG(INFO) << "p1: " << p1->ToString();
   DLOG(INFO) << "p2: " << p2->ToString();
   DLOG(INFO) << "g1: " << g1->ToString();
   DLOG(INFO) << "g2: " << g2->ToString();

   ASSERT_NO_FATAL_FAILURE(SerialSweep());

   // p1 should be:
   ASSERT_TRUE(IsOld(p1));
   ASSERT_TRUE(IsMarked(p1));
   ASSERT_TRUE(IsWord(p1, kPtr1Value));
   // p1 shouldn't be:
   ASSERT_FALSE(IsNew(p1));
   ASSERT_FALSE(IsRemembered(p1));
   ASSERT_FALSE(IsForwarding(p1));

   // p2 should be:
   ASSERT_TRUE(IsOld(p2));
   ASSERT_TRUE(IsMarked(p2));
   ASSERT_TRUE(IsWord(p2, kPtr2Value));
   // p2 shouldn't be:
   ASSERT_FALSE(IsNew(p2));
   ASSERT_FALSE(IsRemembered(p2));
   ASSERT_FALSE(IsForwarding(p2));

   // g1 should be:
   ASSERT_TRUE(IsOld(g1));
   ASSERT_TRUE(IsUnallocated(g2));
   // g1 shouldn't be:
   ASSERT_FALSE(IsNew(g1));
   ASSERT_FALSE(IsMarked(g1));
   ASSERT_FALSE(IsRemembered(g1));
   ASSERT_FALSE(IsForwarding(g1));

   // g2 should be:
   ASSERT_TRUE(IsOld(g2));
   ASSERT_TRUE(IsUnallocated(g2));
   // g2 shouldn't be:
   ASSERT_FALSE(IsNew(g2));
   ASSERT_FALSE(IsMarked(g2));
   ASSERT_FALSE(IsRemembered(g2));
   ASSERT_FALSE(IsForwarding(g2));
 }
}