#ifndef POSEIDON_TEST_OLD_PAGE_H
#define POSEIDON_TEST_OLD_PAGE_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "poseidon/flags.h"
#include "poseidon/heap/old_page.h"

namespace poseidon{
 using namespace ::testing;

 class OldPageTest : public Test {
  protected:
   OldPageTest() = default;
  public:
   ~OldPageTest() override = default;
 };
}

#endif//POSEIDON_TEST_OLD_PAGE_H