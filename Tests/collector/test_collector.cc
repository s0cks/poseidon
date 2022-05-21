#include <gtest/gtest.h>
#include <glog/logging.h>

#include "helpers.h"
#include "poseidon/local.h"
#include "poseidon/collector/collector.h"
#include "poseidon/allocator/allocator.h"

namespace poseidon{
 using namespace ::testing;

 static inline Local<word>
 AllocateRootWord(word value){
   auto raw_ptr = (RawObject*)Allocator::Allocate(sizeof(word));
   *((word*)raw_ptr->GetPointer()) = value;

   Local<word> handle;
   handle = raw_ptr->GetAddress();
   return handle;
 }

 static inline RawObject*
 AllocateWord(word value){
   auto raw_ptr = (RawObject*)Allocator::Allocate(sizeof(word));
   *((word*)raw_ptr->GetPointer()) = value;
   return raw_ptr;
 }

 static const constexpr int64_t kNumberOfRoots = 3;
 static Local<word> kRoots[kNumberOfRoots];

 class CollectorTest : public Test{
  public:
   static void
   SetUpTestCase(){
     // reset allocation context
     Heap::ResetCurrentThreadHeap();
     LocalPage::ResetLocalPageForCurrentThread();

     // allocate root set
     for(auto idx = 0; idx < kNumberOfRoots; idx++){
       auto& root = kRoots[idx] = AllocateRootWord(idx);
       // Check that the root is properly initialized
       ASSERT_EQ(*root.Get(), idx);
       ASSERT_TRUE(IsNew(root));
       ASSERT_FALSE(IsOld(root));
       ASSERT_FALSE(IsMarked(root));
       ASSERT_FALSE(IsRemembered(root));
       ASSERT_FALSE(IsForwarding(root));
     }
   }

   static void
   TearDownTestCase(){

   }
  public:
   CollectorTest() = default;
   ~CollectorTest() override = default;
 };

 TEST_F(CollectorTest, TestFirstMinorCollection){
   Collector::MinorCollection();
   // Check that the roots have survived the minor collection
   for(auto idx = 0; idx < kNumberOfRoots; idx++){
     auto& root = kRoots[idx];
     ASSERT_EQ(*root.Get(), idx);
     ASSERT_TRUE(IsNew(root));
     ASSERT_FALSE(IsOld(root));
     ASSERT_FALSE(IsMarked(root));
     ASSERT_TRUE(IsRemembered(root)); // the roots should be remembered now since they have survived a minor collection.
     ASSERT_FALSE(IsForwarding(root));
   }
 }

 TEST_F(CollectorTest, TestSecondMinorCollection){
   Collector::MinorCollection();
   // Check that the roots have survived the minor collection but have been promoted
   for(auto idx = 0; idx < kNumberOfRoots; idx++){
     auto& root = kRoots[idx];
     ASSERT_EQ(*root.Get(), idx);
     ASSERT_FALSE(IsNew(root)); // the roots should no longer be new objects
     ASSERT_TRUE(IsOld(root)); // the roots should be old now
     ASSERT_FALSE(IsMarked(root));
     ASSERT_TRUE(IsRemembered(root)); // the roots should still be remembered
     ASSERT_FALSE(IsForwarding(root));
   }
 }

 TEST_F(CollectorTest, TestMajorCollection){

 }
}