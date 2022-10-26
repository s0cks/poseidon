#ifndef POSEIDON_TEST_SEMISPACE_H
#define POSEIDON_TEST_SEMISPACE_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "poseidon/heap/semispace.h"

namespace poseidon{
 using namespace ::testing;

 class SemispaceTest : public Test {
  protected:
   SemispaceTest() = default;
  public:
   ~SemispaceTest() override = default;
 };
}


#endif//POSEIDON_TEST_SEMISPACE_H