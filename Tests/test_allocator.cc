#include "test_allocator.h"
#include "local.h"
#include "allocator.h"

namespace poseidon{
  TEST(AllocatorTest, TestAllocate){
    poseidon::Local<uint64_t> a = poseidon::Allocator::AllocateLocal<uint64_t>(10);
    ASSERT_EQ(*a.Get(), 10);

    poseidon::Local<uint64_t> b = poseidon::Allocator::AllocateLocal<uint64_t>(100);
    ASSERT_EQ(*b.Get(), 100);
  }
}