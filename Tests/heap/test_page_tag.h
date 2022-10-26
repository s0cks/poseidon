#ifndef POSEIDON_TEST_PAGE_TAG_H
#define POSEIDON_TEST_PAGE_TAG_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "poseidon/heap/page_tag.h"

namespace poseidon {
 using namespace ::testing;

 class PageTagTest : public Test {
  protected:
   PageTagTest() = default;
  public:
   ~PageTagTest() override = default;
 };
}

#endif // POSEIDON_TEST_PAGE_TAG_H