#include "heap/test_semispace.h"

#include "poseidon/flags.h"
#include "poseidon/object.h"

#include "matchers/is_pointer_to.h"
#include "mock_raw_object_visitor.h"
#include "assertions/ptr_assertions.h"
#include "assertions/type_assertions.h"

namespace poseidon {
 using namespace ::testing;

 TEST_F(SemispaceTest, TestVisitPointers_WillPass_VisitsNothing) {
   MockRawObjectVisitor visitor;
   ON_CALL(visitor, Visit(_))
       .WillByDefault(Return(false));
   ASSERT_TRUE(semispace().VisitPointers(&visitor));
 }

 TEST_F(SemispaceTest, TestVisitPointers_WillPass_VisitsOneNull) {
   MockRawObjectVisitor visitor;

   auto a = Null::TryAllocateIn(&semispace());
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsNull(a));
   EXPECT_CALL(visitor, Visit(IsPointerTo(a)));

   ASSERT_TRUE(semispace().VisitPointers(&visitor));
 }

 TEST_F(SemispaceTest, TestVisitPointers_WillPass_VisitsOneTrue) {
   MockRawObjectVisitor visitor;

   auto a = Bool::TryAllocateIn(&semispace(), true);
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsBool(a));
   ASSERT_TRUE(BoolEq(true, a));
   EXPECT_CALL(visitor, Visit(IsPointerTo(a)));

   ASSERT_TRUE(semispace().VisitPointers(&visitor));
 }

 TEST_F(SemispaceTest, TestVisitPointers_WillPass_VisitsOneFalse) {
   MockRawObjectVisitor visitor;

   auto a = Bool::TryAllocateIn(&semispace(), false);
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsBool(a));
   ASSERT_TRUE(BoolEq(false, a));
   EXPECT_CALL(visitor, Visit(IsPointerTo(a)));

   ASSERT_TRUE(semispace().VisitPointers(&visitor));
 }

#define DEFINE_VISIT_POINTERS_NUMBER_TYPE_PASSES_NEW_ZONE_TEST(Type) \
 TEST_F(SemispaceTest, TestVisitPointers_WillPass_VisitsOne##Type) {   \
   MockRawObjectVisitor visitor;                                     \
   static constexpr const Raw##Type kAValue = 0xA;                   \
   auto a = Type::TryAllocateIn(&semispace(), kAValue);              \
   ASSERT_TRUE(IsAllocated(a));                                      \
   ASSERT_TRUE(Is##Type(a));                                         \
   ASSERT_TRUE(Type##Eq(kAValue, a));                                \
   EXPECT_CALL(visitor, Visit(IsPointerTo(a)));                      \
   ASSERT_TRUE(semispace().VisitPointers(&visitor));                 \
 }
 FOR_EACH_INT_TYPE(DEFINE_VISIT_POINTERS_NUMBER_TYPE_PASSES_NEW_ZONE_TEST);
#undef DEFINE_VISIT_POINTERS_NUMBER_TYPE_PASSES_NEW_ZONE_TEST

 TEST_F(SemispaceTest, TestVisitPointers_WillPass_VisitsMultipleNulls) {
   MockRawObjectVisitor visitor;

   auto a = Null::TryAllocateIn(&semispace());
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsNull(a));
   EXPECT_CALL(visitor, Visit(IsPointerTo(a)));

   auto b = Null::TryAllocateIn(&semispace());
   ASSERT_TRUE(IsAllocated(b));
   ASSERT_TRUE(IsNull(b));
   EXPECT_CALL(visitor, Visit(IsPointerTo(b)));

   ASSERT_TRUE(semispace().VisitPointers(&visitor));
 }

 TEST_F(SemispaceTest, TestVisitPointers_WillPass_VisitsMultipleTrues) {
   MockRawObjectVisitor visitor;

   auto a = Bool::TryAllocateIn(&semispace(), true);
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsBool(a));
   ASSERT_TRUE(BoolEq(true, a));
   EXPECT_CALL(visitor, Visit(IsPointerTo(a)));

   auto b = Bool::TryAllocateIn(&semispace(), true);
   ASSERT_TRUE(IsAllocated(b));
   ASSERT_TRUE(IsBool(b));
   ASSERT_TRUE(BoolEq(true, b));
   EXPECT_CALL(visitor, Visit(IsPointerTo(b)));

   ASSERT_TRUE(semispace().VisitPointers(&visitor));
 }

 TEST_F(SemispaceTest, TestVisitPointers_WillPass_VisitsMultipleFalses) {
   MockRawObjectVisitor visitor;

   auto a = Bool::TryAllocateIn(&semispace(), false);
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsBool(a));
   ASSERT_TRUE(BoolEq(false, a));
   EXPECT_CALL(visitor, Visit(IsPointerTo(a)));

   auto b = Bool::TryAllocateIn(&semispace(), false);
   ASSERT_TRUE(IsAllocated(b));
   ASSERT_TRUE(IsBool(b));
   ASSERT_TRUE(BoolEq(false, b));
   EXPECT_CALL(visitor, Visit(IsPointerTo(b)));

   ASSERT_TRUE(semispace().VisitPointers(&visitor));
 }

#define DEFINE_VISIT_MULTIPLE_POINTERS_NUMBER_TYPE_PASSES_NEW_ZONE_TEST(Type) \
 TEST_F(SemispaceTest, TestVisitPointers_WillPass_VisitsMultiple##Type##s) {  \
   MockRawObjectVisitor visitor;                                              \
   static constexpr const Raw##Type kAValue = 0xA;                            \
   auto a = Type::TryAllocateIn(&semispace(), kAValue);                       \
   ASSERT_TRUE(IsAllocated(a));                                               \
   ASSERT_TRUE(Is##Type(a));                                                  \
   ASSERT_TRUE(Type##Eq(kAValue, a));                                         \
   EXPECT_CALL(visitor, Visit(IsPointerTo(a)));                               \
   static constexpr const Raw##Type kBValue = 0xB;                            \
   auto b = Type::TryAllocateIn(&semispace(), kBValue);                       \
   ASSERT_TRUE(IsAllocated(b));                                               \
   ASSERT_TRUE(Is##Type(b));                                                  \
   ASSERT_TRUE(Type##Eq(kBValue, b));                                         \
   EXPECT_CALL(visitor, Visit(IsPointerTo(b)));                               \
   ASSERT_TRUE(semispace().VisitPointers(&visitor));                          \
 }

 FOR_EACH_INT_TYPE(DEFINE_VISIT_MULTIPLE_POINTERS_NUMBER_TYPE_PASSES_NEW_ZONE_TEST)
#undef DEFINE_VISIT_MULTIPLE_POINTERS_NUMBER_TYPE_PASSES_NEW_ZONE_TEST
}