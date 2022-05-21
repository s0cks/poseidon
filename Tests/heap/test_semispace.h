#ifndef POSEIDON_TEST_SEMISPACE_H
#define POSEIDON_TEST_SEMISPACE_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "memory_region_test.h"
#include "poseidon/heap/semispace.h"

namespace poseidon{
 using namespace ::testing;

 class SemispaceTest : public MemoryRegionTest{
  protected:
   Semispace semispace_;
  public:
   SemispaceTest():
     MemoryRegionTest(NewZone::GetDefaultSemispaceSize()),
     semispace_(region()){
   }
   ~SemispaceTest() override = default;
 };
}


#endif//POSEIDON_TEST_SEMISPACE_H