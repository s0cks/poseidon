#include <gtest/gtest.h>
#include <glog/logging.h>

#include "helpers.h"
#include "poseidon/compactor.h"

namespace poseidon{
 class CompactorTest : public ::testing::Test{
  public:
   CompactorTest() = default;
   ~CompactorTest() override = default;
 };

 TEST_F(CompactorTest, TestCompact){ //TODO: better RawObject assertions.
   static constexpr const uint64_t kNumberOfTestObjects = 128;
   static constexpr const uint64_t kValue = 100;

   auto page = new HeapPage(10 * 1024);

   // fill the heap page
   for(auto idx = 0; idx < kNumberOfTestObjects; idx++)
     ASSERT_TRUE(page->Allocate(Class::CLASS_INT->GetAllocationSize()) != nullptr);

   // create a new local in the heap page to be saved during compaction.
   auto val = NewLocalInt(page, kValue);
   // check that the local was allocated correctly.
   ASSERT_EQ(val->Get(), kValue);
   ASSERT_EQ(val.GetRawObjectPointer()->GetPointerSize(), Class::CLASS_INT->GetAllocationSize());
   ASSERT_TRUE(val.GetRawObjectPointer()->IsOld());
   ASSERT_FALSE(val.GetRawObjectPointer()->IsNew());
   ASSERT_FALSE(val.GetRawObjectPointer()->IsRemembered());
   ASSERT_FALSE(val.GetRawObjectPointer()->IsMarked());
   ASSERT_FALSE(val.GetRawObjectPointer()->IsForwarding());

   // cache the old address. since a bunch of garbage is allocated before the local,
   // the old address **will always be different** from the new address.
   auto old_address = val.GetRawObjectPointer()->GetAddress();

   Compactor compactor(page);
   compactor.Compact();

   // check that the object has been moved.
   ASSERT_NE(old_address, val.GetRawObjectPointer()->GetAddress());
   // check that the page has been updated.
//TODO:ASSERT_EQ(page->GetCurrentAddress(), page->GetStartingAddress() + val.GetRawObjectPointer()->GetTotalSize());
//TODO:ASSERT_EQ(page->GetNumberOfBytesAllocated(), val.GetRawObjectPointer()->GetTotalSize());

   // check that the object's state has been preserved.
   ASSERT_EQ(val->Get(), kValue);
   ASSERT_EQ(val.GetRawObjectPointer()->GetPointerSize(), Class::CLASS_INT->GetAllocationSize());
   ASSERT_TRUE(val.GetRawObjectPointer()->IsOld());
   ASSERT_FALSE(val.GetRawObjectPointer()->IsNew());
   ASSERT_FALSE(val.GetRawObjectPointer()->IsRemembered());
   ASSERT_FALSE(val.GetRawObjectPointer()->IsMarked());
   ASSERT_FALSE(val.GetRawObjectPointer()->IsForwarding());
 }
}