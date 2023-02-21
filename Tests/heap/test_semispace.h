#ifndef PSDN_TEST_SEMISPACE_H
#define PSDN_TEST_SEMISPACE_H

#include "base_memory_region_test.h"

#include "poseidon/local/local.h"
#include "poseidon/marker/marker.h"
#include "poseidon/heap/semispace.h"

namespace poseidon {
 class SemispaceTest : public BaseMemoryRegionTest {
  protected:
   static constexpr const Size kTestRegionSize = Megabytes(8);
   static constexpr const Size kSemispaceSize = kTestRegionSize;

   Semispace semispace_;
   Marker marker_;

   SemispaceTest():
     BaseMemoryRegionTest((RegionSize)kTestRegionSize),
     semispace_(region()),
     marker_() {
   }

   inline Semispace& semispace() {
     return semispace_;
   }

   inline Marker& marker() {
     return marker_;
   }

   inline void Mark(Pointer* ptr) {
     ASSERT_TRUE(marker().Visit(ptr));
   }

   template<class T>
   inline void Mark(Local<T>& local) {
     return Mark(local.raw_ptr());
   }

   template<class T>
   inline void Mark(T* value) {
     return Mark(value->raw_ptr());
   }

   inline Pointer*
   TryAllocatePointer(const ObjectSize size) {
     return semispace().TryAllocatePointer(size);
   }

   inline void ClearSemispace() {
     return semispace().Clear();
   }

   inline void SetCurrentAddress(const uword address) {
     semispace().current_ = address;
   }
  public:
   ~SemispaceTest() override = default;

   void SetUp() override {
     BaseMemoryRegionTest::SetUp();
     ASSERT_NO_FATAL_FAILURE(semispace().SetWritable());
     ASSERT_NO_FATAL_FAILURE(semispace().Clear());
#ifdef PSDN_DEBUG
     ASSERT_NO_FATAL_FAILURE(semispace().SetReadOnly());
     ASSERT_NO_FATAL_FAILURE(SemispacePrinter::Print<>(&semispace()));
#endif //PSDN_DEBUG
     ASSERT_NO_FATAL_FAILURE(semispace().SetWritable());
   }

   void TearDown() override {
     BaseMemoryRegionTest::TearDown();
     ASSERT_NO_FATAL_FAILURE(semispace().SetReadOnly());
#ifdef PSDN_DEBUG
     ASSERT_NO_FATAL_FAILURE(SemispacePrinter::Print<>(&semispace()));
#endif //PSDN_DEBUG
   }
 };
}

#endif //PSDN_TEST_SEMISPACE_H