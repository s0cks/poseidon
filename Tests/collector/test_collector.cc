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
}