#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <glog/logging.h>

#include "helpers.h"
#include "poseidon/allocator.h"

namespace poseidon{
 using namespace ::testing;

 class AllocatorTest : public Test{
  public:
   AllocatorTest() = default;
   ~AllocatorTest() override = default;
 };

 TEST_F(AllocatorTest, TestAllocateNewZone){
   auto val = Allocator::New<int>();
   (*val) = 100;

   ASSERT_EQ((*val), 100);
 }
}