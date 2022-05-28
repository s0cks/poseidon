#include "helpers/assertions.h"
#include "collector/test_sweeper.h"

namespace poseidon{
 TEST_F(SweeperTest, TestSweepObject){
   static constexpr const int64_t kPtr1Value = 10;
   auto p1 = TryAllocateMarkedWord(zone(), kPtr1Value);
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
   static constexpr const int64_t kRoot1Value = 222;
   auto r1 = Local<word>();
   r1 = TryAllocateMarkedWord(zone(), kRoot1Value)->GetAddress();
   ASSERT_TRUE(IsMarkedWord(r1, kRoot1Value));

   static constexpr const int64_t kRoot2Value = 333;
   auto r2 = Local<word>();
   r2 = TryAllocateMarkedWord(zone(), kRoot2Value)->GetAddress();
   ASSERT_TRUE(IsMarkedWord(r2, kRoot2Value));

   static constexpr const int64_t kRoot3Value = 444;
   auto r3 = Local<word>();
   r3 = TryAllocateMarkedWord(zone(), kRoot3Value)->GetAddress();
   ASSERT_TRUE(IsMarkedWord(r3, kRoot3Value));

   ASSERT_NO_FATAL_FAILURE(SerialSweep());

   ASSERT_TRUE(IsMarkedWord(r1, kRoot1Value));
   ASSERT_TRUE(IsMarkedWord(r2, kRoot2Value));
   ASSERT_TRUE(IsMarkedWord(r3, kRoot3Value));
 }
}