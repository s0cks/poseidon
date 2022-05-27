#ifndef POSEIDON_MEMORY_REGION_TEST_H
#define POSEIDON_MEMORY_REGION_TEST_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "poseidon/heap/heap.h"
#include "poseidon/heap/new_zone.h"
#include "poseidon/heap/old_zone.h"
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

 class BaseNewZoneTest : public MemoryRegionTest{
  protected:
   NewZone zone_;

   explicit BaseNewZoneTest(int64_t size = GetNewZoneSize()):
     MemoryRegionTest(size),
     zone_(region()){
   }

   inline NewZone* zone(){
     return &zone_;
   }
  public:
   ~BaseNewZoneTest() override = default;
 };

 class BaseOldZoneTest : public MemoryRegionTest{
  protected:
   OldZone zone_;

   explicit BaseOldZoneTest(int64_t size = GetOldZoneSize(), int64_t page_size = GetOldPageSize()):
     MemoryRegionTest(size),
     zone_(region(), size, page_size){
   }
  public:
   ~BaseOldZoneTest() override = default;
 };

 class HeapTest : public MemoryRegionTest{
  protected:
   Heap heap_;

   explicit HeapTest(int64_t size = GetTotalInitialHeapSize()):
     MemoryRegionTest(size),
     heap_(region()){
   }

   inline Heap* heap(){
     return &heap_;
   }

   inline NewZone* new_zone(){
     return heap()->new_zone();
   }

   inline OldZone* old_zone(){
     return heap()->old_zone();
   }
  public:
   ~HeapTest() override = default;
 };
}

#endif//POSEIDON_MEMORY_REGION_TEST_H