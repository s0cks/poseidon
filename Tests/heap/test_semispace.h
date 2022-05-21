#ifndef POSEIDON_TEST_SEMISPACE_H
#define POSEIDON_TEST_SEMISPACE_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "poseidon/heap/semispace.h"

namespace poseidon{
 using namespace ::testing;

 class SemispaceTest : public Test{
  public:
   static const constexpr int64_t kDefaultSemispaceSize = 2 * kMB;
  protected:
   MemoryRegion region_;
   Semispace semispace_;

   inline MemoryRegion* region(){
     return &region_;
   }

   void SetUp() override{
     ASSERT_TRUE(region()->Protect(MemoryRegion::kReadWrite)) << "cannot set " << region_ << " to " << MemoryRegion::kReadWrite;
   }
  public:
   SemispaceTest():
     region_(kDefaultSemispaceSize),
     semispace_(region()){
   }
   ~SemispaceTest() override = default;
 };
}


#endif//POSEIDON_TEST_SEMISPACE_H