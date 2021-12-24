#ifndef POSEIDON_TEST_ALLOCATOR_H
#define POSEIDON_TEST_ALLOCATOR_H

#include <gtest/gtest.h>

namespace poseidon{
  class AllocatorTest : public ::testing::Test{
   public:
    AllocatorTest() = default;
    ~AllocatorTest() override = default;
  };
}

#endif //POSEIDON_TEST_ALLOCATOR_H