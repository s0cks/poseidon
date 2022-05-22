#ifndef POSEIDON_TEST_SWEEPER_H
#define POSEIDON_TEST_SWEEPER_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <glog/logging.h>

#include "helpers.h"
#include "memory_region_test.h"
#include "poseidon/collector/sweeper.h"

namespace poseidon{
 using namespace ::testing;

 class MockFreeList : public FreeList{
  public:
   MockFreeList(uword start, int64_t size):
     FreeList(start, size){
   }
   MockFreeList(MemoryRegion* region, int64_t offset, int64_t size):
     MockFreeList(region->GetStartingAddress() + offset, size){
   }
   MockFreeList(MemoryRegion* region, int64_t size):
     MockFreeList(region, 0, size){
   }

   explicit MockFreeList(MemoryRegion* region):
     MockFreeList(region, region->size()){
   }
   ~MockFreeList() override = default;
 };

 class SweeperTest : public MemoryRegionTest{
  protected:
   MockFreeList free_list_;
   OldZone zone_;

   inline OldZone* zone(){
     return &zone_;
   }

   inline FreeList* free_list(){
     return &free_list_;
   }

   inline void
   SweepObject(RawObject* ptr){
     return Sweeper::SweepObject(free_list(), ptr);
   }

   inline void
   SerialSweep(){
     return Sweeper::SerialSweep(zone());
   }

   inline void
   ParallelSweep(){
     return Sweeper::ParallelSweep(zone());
   }
  public:
   SweeperTest():
     MemoryRegionTest(GetOldZoneSize()),
     free_list_(region()),
     zone_(region(), GetOldPageSize(), free_list()){
   }
   ~SweeperTest() override = default;
 };

 class ParallelSweeperTest : public Test{
  public:
   ParallelSweeperTest() = default;
   ~ParallelSweeperTest() override = default;
 };
}


#endif//POSEIDON_TEST_SWEEPER_H