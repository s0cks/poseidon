#include <gtest/gtest.h>

#include "helpers.h"
#include "poseidon/pointer.h"

#include "poseidon/type/int.h"
#include "assertions/int_assertions.h"

namespace poseidon{
 using namespace ::testing;

 class PointerTest : public Test {
  protected:
   MemoryRegion region_;
   NewZone* zone_;

   PointerTest():
    Test(),
    region_(GetNewZoneSize()),
    zone_(NewZone::New(region_)) {
   }

   inline MemoryRegion region() const {
     return region_;
   }

   inline NewZone* zone() const {
     return zone_;
   }
  public:
   ~PointerTest() override {
     delete zone_;
   }

   void SetUp() override {
     ASSERT_TRUE(region_.Protect(MemoryRegion::kReadWrite));
   }
 };

 TEST_F(PointerTest, TestTryAllocateIn_WillPass) {
   auto i1 = Int::TryAllocateIn<>(zone());
   ASSERT_NE(i1, nullptr);
   ASSERT_TRUE(IsInt(i1->raw_ptr()));
   ASSERT_TRUE(IntEq((const RawInt) 0, i1));
 }
}