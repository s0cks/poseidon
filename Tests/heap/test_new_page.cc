#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "poseidon/heap/new_page.h"

#include "helpers.h"

namespace poseidon {
#define UNALLOCATED 0 //TODO: cleanup

 using namespace ::testing;

 class NewPageTest : public Test {
  protected:
   NewPageTest() = default;
  public:
   ~NewPageTest() override = default;
 };
}