#include "helpers.h"
#include "poseidon/heap/semispace.h"
#include "collector/test_scavenger.h"

namespace poseidon{
 TEST_F(ScavengerTest, TestScavengeObject){
   Semispace tospace(new_zone()->tospace(), new_zone()->semisize());

   static constexpr const int64_t kRootValue = 222;
   auto old_ptr = TryAllocateNewWord(new_zone(), kRootValue);
   ASSERT_TRUE(IsAllocated(old_ptr));
   ASSERT_TRUE(IsNew(old_ptr));
   ASSERT_TRUE(IsWord(old_ptr, kRootValue));

   auto new_ptr = ScavengeObject(old_ptr);
   ASSERT_TRUE(IsAllocated(new_ptr));
   ASSERT_TRUE(IsNew(new_ptr));
   ASSERT_TRUE(IsWord(new_ptr, kRootValue));
   ASSERT_NE(old_ptr->GetAddress(), new_ptr->GetAddress());
 }

 TEST_F(ScavengerTest, TestPromoteObject){
   static constexpr const int64_t kRootValue = 222;
   auto old_ptr = TryAllocateNewWord(new_zone(), kRootValue);
   ASSERT_TRUE(IsAllocated(old_ptr));
   ASSERT_TRUE(IsNew(old_ptr));
   ASSERT_TRUE(IsWord(old_ptr, kRootValue));

   auto new_ptr = PromoteObject(old_ptr);
   ASSERT_TRUE(IsAllocated(new_ptr));
   ASSERT_TRUE(IsOld(new_ptr));
   ASSERT_TRUE(IsWord(new_ptr, kRootValue));
   ASSERT_NE(old_ptr->GetAddress(), new_ptr->GetAddress());
 }

 TEST_F(ScavengerTest, TestProcessObject){
   Semispace tospace(new_zone()->tospace(), new_zone()->semisize());

   static constexpr const int64_t kRootValue = 222;
   auto p0 = TryAllocateNewWord(new_zone(), kRootValue);
   ASSERT_TRUE(IsAllocated(p0));
   ASSERT_TRUE(IsNew(p0));
   ASSERT_TRUE(IsWord(p0, kRootValue));

   auto p1 = ProcessObject(p0);
   ASSERT_TRUE(IsForwardingTo(p0, p1));
   ASSERT_TRUE(IsAllocated(p1));
   ASSERT_TRUE(IsNew(p1));
   ASSERT_TRUE(IsRemembered(p1));
   ASSERT_TRUE(IsWord(p1, kRootValue));
   ASSERT_NE(p1->GetAddress(), p0->GetAddress());

   auto p2 = ProcessObject(p1);
   ASSERT_TRUE(IsForwardingTo(p1, p2));
   ASSERT_TRUE(IsAllocated(p2));
   ASSERT_TRUE(IsOld(p2));
   ASSERT_TRUE(IsRemembered(p2));
   ASSERT_TRUE(IsWord(p2, kRootValue));
   ASSERT_NE(p2->GetAddress(), p0->GetAddress());
   ASSERT_NE(p2->GetAddress(), p1->GetAddress());
 }

 TEST_F(ScavengerTest, TestSerialScavenge){
   LocalPage::ResetLocalPageForCurrentThread();

   static constexpr const int64_t kRoot1Value = 222;
   auto r1 = Local<word>();
   r1 = TryAllocateNewWord(new_zone(), kRoot1Value)->GetAddress();
   ASSERT_TRUE(IsAllocated(r1));
   ASSERT_TRUE(IsNew(r1));
   ASSERT_TRUE(IsWord(r1, kRoot1Value));

   static constexpr const int64_t kGarbage1Value = 111;
   auto g1 = TryAllocateNewWord(new_zone(), kGarbage1Value);
   ASSERT_TRUE(IsAllocated(g1));
   ASSERT_TRUE(IsNew(g1));
   ASSERT_TRUE(IsWord(g1, kGarbage1Value));

   ASSERT_NO_FATAL_FAILURE(SerialScavenge());

   ASSERT_TRUE(IsAllocated(r1));
   ASSERT_TRUE(IsNew(r1));
   ASSERT_TRUE(IsWord(r1, kRoot1Value));

   ASSERT_TRUE(IsUnallocated(g1));
 }

 TEST_F(ScavengerTest, TestParallelScavenge){
   LocalPage::ResetLocalPageForCurrentThread();

   static constexpr const int64_t kRoot1Value = 222;
   auto r1 = Local<word>();
   r1 = TryAllocateNewWord(new_zone(), kRoot1Value)->GetAddress();
   ASSERT_TRUE(IsAllocated(r1));
   ASSERT_TRUE(IsNew(r1));
   ASSERT_TRUE(IsWord(r1, kRoot1Value));

   static constexpr const int64_t kGarbage1Value = 111;
   auto g1 = TryAllocateNewWord(new_zone(), kGarbage1Value);
   ASSERT_TRUE(IsAllocated(g1));
   ASSERT_TRUE(IsNew(g1));
   ASSERT_TRUE(IsWord(g1, kGarbage1Value));

   ASSERT_NO_FATAL_FAILURE(ParallelScavenge());

   ASSERT_TRUE(IsAllocated(r1));
   ASSERT_TRUE(IsNew(r1));
   ASSERT_TRUE(IsWord(r1, kRoot1Value));

   ASSERT_TRUE(IsUnallocated(g1));
 }
}