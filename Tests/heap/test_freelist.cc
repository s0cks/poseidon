#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "poseidon/flags.h"
#include "poseidon/heap/freelist.h"

namespace poseidon {
 using namespace ::testing;

 class FreeListTest : public Test {
  protected:
   FreeListTest() = default;
  public:
   ~FreeListTest() override = default;
 };

 TEST_F(FreeListTest, TestConstructor) {
   MemoryRegion region(GetOldZoneSize());
   FreeList free_list(region);
 }

 TEST_F(FreeListTest, TestEquals) {

 }
}