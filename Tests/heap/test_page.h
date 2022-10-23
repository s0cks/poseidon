#ifndef POSEIDON_TEST_PAGE_H
#define POSEIDON_TEST_PAGE_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "poseidon/heap/page.h"

namespace poseidon {
 using namespace ::testing;

 class PageTagTest : public Test {
  protected:
   PageTagTest() = default;
  public:
   ~PageTagTest() override = default;
 };

 class PageTest : public Test {
  protected:
   PageTest() = default;
  public:
   ~PageTest() override = default;
 };
}

#endif // POSEIDON_TEST_PAGE_H