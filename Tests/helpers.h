#ifndef POSEIDON_HELPERS_H
#define POSEIDON_HELPERS_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <glog/logging.h>

#include "poseidon/heap/heap.h"

#include "helpers/assertions.h"

namespace poseidon{
 using namespace ::testing;

 class MockFreeListNodeVisitor : public FreeObjectVisitor {
  public:
   MockFreeListNodeVisitor():
       FreeObjectVisitor() {
     ON_CALL(*this, Visit)
      .WillByDefault([](FreeObject* val) {
        return true;
      });
   }
   ~MockFreeListNodeVisitor() override = default;
   MOCK_METHOD(bool, Visit, (FreeObject*), (override));
 };
}

#define DEFINE_TRY_ALLOCATE_BYTES_FAILS_TEST(TestSuite, TestName, Zone, ZoneSize, NumberOfBytes) \
  TEST_F(TestSuite, TestTryAllocateBytes_WillFail_##TestName) {                                  \
    MemoryRegion region(ZoneSize);                                                               \
    ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));                                       \
    Zone zone(region);                                                                           \
    auto new_ptr = zone.TryAllocateBytes(NumberOfBytes);                                         \
    ASSERT_EQ(new_ptr, UNALLOCATED);                                                             \
  }

#endif //POSEIDON_HELPERS_H
