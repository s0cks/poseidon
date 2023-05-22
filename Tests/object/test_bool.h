#ifndef POSEIDON_TEST_BOOL_H
#define POSEIDON_TEST_BOOL_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "poseidon/object.h"

namespace poseidon {
 class BoolTest : public ::testing::Test {
  public:
   BoolTest() = default;
   ~BoolTest() override = default;
 };
}

#endif //POSEIDON_TEST_BOOL_H
