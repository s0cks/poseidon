#ifndef POSEIDON_TEST_FREELIST_H
#define POSEIDON_TEST_FREELIST_H

#include <gtest/gtest.h>

#include "poseidon/marker/marker.h"

#include "poseidon/freelist/freelist.h"

#ifdef PSDN_DEBUG
#include "poseidon/freelist/freelist_printer.h"
#endif //PSDN_DEBUG

namespace poseidon {

 class FreeListTest : public testing::Test {
  private:
   MemoryRegion test_region_;
   FreeList free_list_;
  protected:
   FreeListTest():
     Test(),
     test_region_(flags::GetOldZoneSize(), MemoryRegion::kReadWrite),
     free_list_(test_region_) {
   }

   inline MemoryRegion& test_region() {
     return test_region_;
   }

   inline FreeList& free_list() {
     return free_list_;
   }

   inline void Insert(const Region& region) {
     return free_list().Insert(region);
   }

   inline void Insert(const uword start, const RegionSize size) {
     return Insert({ start, size });
   }

   inline FreePointer* FindBestFit(const RegionSize size) {
     return free_list().FindBestFit(size);
   }

   inline void
   Remove(const Region& region) {
     return free_list().Remove(region);
   }

   static inline uword
   TryAllocateBytes(FreeList& free_list, const ObjectSize size) {
     return free_list.TryAllocateBytes(size);
   }

   static inline Pointer*
   TryAllocateWord(FreeList& free_list, word value) {
     auto address = TryAllocateBytes(free_list, kWordSize);
     if (address == UNALLOCATED)
       return nullptr;
     auto ptr = (Pointer*)address;
     (*((word*)ptr->GetObjectPointerAddress())) = value;
     return ptr;
   }

   static inline Pointer*
   TryAllocateMarkedWord(FreeList& free_list, word value) {
     auto address = TryAllocateBytes(free_list, kWordSize);
     if (address == UNALLOCATED)
       return nullptr;
     Marker marker;
     auto ptr = (Pointer*)address;
     (*((word*)ptr->GetObjectPointerAddress())) = value;
     marker.Visit(ptr);
     return ptr;
   }
  public:
   ~FreeListTest() override = default;

   void SetUp() override {
     ASSERT_TRUE(test_region_.Protect(MemoryRegion::kReadWrite));
     ASSERT_NO_FATAL_FAILURE(test_region_.ClearRegion());
     ASSERT_NO_FATAL_FAILURE(free_list_.ClearFreeList());
     ASSERT_TRUE(test_region().Protect(MemoryRegion::kReadOnly));
#ifdef PSDN_DEBUG
     DLOG(INFO) << "Freelist (Before):";
     FreeListPrinter::Print(&free_list());
#endif //PSDN_DEBUG
     ASSERT_TRUE(test_region().Protect(MemoryRegion::kReadWrite));
   }

   void TearDown() override {
#ifdef PSDN_DEBUG
     DLOG(INFO) << "Freelist (After):";
     FreeListPrinter::Print(&free_list());
#endif //PSDN_DEBUG
     ASSERT_TRUE(test_region_.Protect(MemoryRegion::kReadOnly));
   }
 };
}

#endif //POSEIDON_TEST_FREELIST_H