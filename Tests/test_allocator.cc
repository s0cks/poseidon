#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <glog/logging.h>

#include "helpers.h"
#include "poseidon/allocator.h"

namespace poseidon{
 class AllocatorTest : public ::testing::Test{
  public:
   AllocatorTest() = default;
   ~AllocatorTest() override = default;
 };

 TEST_F(AllocatorTest, TestAllocateNewZone){
   auto val = Allocator::New<int>();
   (*val) = 100;

   ASSERT_EQ((*val), 100);
 }

 TEST_F(AllocatorTest, TestAllocateLargeObject){
   uint8_t* data = (uint8_t*)Allocator::Allocate(32 * 1024 * 1024);
 }
}