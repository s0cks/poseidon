#include "zone/test_new_zone.h"
#include "poseidon/flags.h"
#include "poseidon/object.h"

#include "matchers/is_pointer_to.h"
#include "mock_raw_object_visitor.h"
#include "assertions/ptr_assertions.h"
#include "assertions/type_assertions.h"

namespace poseidon {
 using namespace ::testing;

 TEST_F(NewZoneTest, TestVisitMarkedPointers_WillPass_VisitsNothing) {
   MockRawObjectVisitor visitor;
   ON_CALL(visitor, Visit(_))
     .WillByDefault(Return(false));
   ASSERT_TRUE(zone().VisitMarkedPointers(&visitor));
 }

 TEST_F(NewZoneTest, TestVisitMarkedPointers_WillPass_VisitsOneInt32s) {
   MockRawObjectVisitor visitor;

   static constexpr const RawInt32 kAValue = 0xA;
   auto a = Int32::TryAllocateIn(&zone(), kAValue);
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsInt32(a));
   ASSERT_TRUE(Int32Eq(kAValue, a));
   ASSERT_TRUE(Mark(a));
   EXPECT_CALL(visitor, Visit(IsPointerTo(a)));

   ASSERT_NO_FATAL_FAILURE(zone().VisitMarkedPointers(&visitor));
 }

 TEST_F(NewZoneTest, TestVisitMarkedPointers_WillPass_VisitsMultipleInt32s) {
   MockRawObjectVisitor visitor;

   static constexpr const RawInt32 kAValue = 0xA;
   auto a = Int32::TryAllocateIn(&zone(), kAValue);
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsInt32(a));
   ASSERT_TRUE(Int32Eq(kAValue, a));
   ASSERT_TRUE(Mark(a));
   EXPECT_CALL(visitor, Visit(IsPointerTo(a)));

   static constexpr const RawInt32 kBValue = 0xB;
   auto b = Int32::TryAllocateIn(&zone(), kBValue);
   ASSERT_TRUE(IsAllocated(b));
   ASSERT_TRUE(IsInt32(b));
   ASSERT_TRUE(Int32Eq(kBValue, b));
   ASSERT_TRUE(Mark(b));
   EXPECT_CALL(visitor, Visit(IsPointerTo(b)));

   ASSERT_NO_FATAL_FAILURE(zone().VisitMarkedPointers(&visitor));
 }
}