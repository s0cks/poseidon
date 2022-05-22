#include <gtest/gtest.h>
#include <glog/logging.h>

#include "helpers.h"
#include "poseidon/local.h"
#include "helpers/assertions.h"
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
   }

   static void
   TearDownTestCase(){

   }
  public:
   CollectorTest() = default;
   ~CollectorTest() override = default;
 };

 TEST_F(CollectorTest, TestMinorCollection){
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
}