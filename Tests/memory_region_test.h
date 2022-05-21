#ifndef POSEIDON_MEMORY_REGION_TEST_H
#define POSEIDON_MEMORY_REGION_TEST_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "poseidon/platform/memory_region.h"

namespace poseidon{
 using namespace ::testing;

 class MemoryRegionTest : public Test{
  protected:
   MemoryRegion region_;

   explicit MemoryRegionTest(int64_t size):
    Test(),
    region_(size){
   }

   inline MemoryRegion* region(){
     return &region_;
   }

   inline void SetProtectionMode(const MemoryRegion::ProtectionMode& mode){
     DLOG(INFO) << "setting " << region_ << " to " << mode;
     ASSERT_TRUE(region()->Protect(mode)) << "cannot set " << region_ << " to " << mode;
   }

   inline void SetReadOnly(){
     return SetProtectionMode(MemoryRegion::kReadOnly);
   }

   inline void SetWriteable(){
     return SetProtectionMode(MemoryRegion::kReadWrite);
   }

   void SetUp() override{
     SetWriteable();
   }
  public:
   ~MemoryRegionTest() override = default;
 };
}

#endif//POSEIDON_MEMORY_REGION_TEST_H