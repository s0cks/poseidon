#include <gtest/gtest.h>
#include <glog/logging.h>

#include "poseidon/freelist.h"
#include "poseidon/memory_region.h"

namespace poseidon{
 using namespace ::testing;

 class FreeListTest : public Test{
  public:
   FreeListTest() = default;
   ~FreeListTest() override = default;
 };

 TEST_F(FreeListTest, TestTryAllocate){
   MemoryRegion region(4 * kMB);
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));

   FreeList freelist(region.GetStartingAddress(), region.size());
   freelist.PrintFreeList();

   auto new_ptr = freelist.TryAllocate(sizeof(uword));
   ASSERT_NE(new_ptr, 0);

   freelist.PrintFreeList();
 }
}