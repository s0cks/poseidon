#ifndef POSEIDON_TEST_NEW_PAGE_H
#define POSEIDON_TEST_NEW_PAGE_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "poseidon/heap/new_page.h"

namespace poseidon {
 using namespace ::testing;

 class NewPageTest : public Test {
  protected:
   NewPageTest() = default;
  public:
   ~NewPageTest() override = default;
 };
}

#endif // POSEIDON_TEST_NEW_PAGE_H