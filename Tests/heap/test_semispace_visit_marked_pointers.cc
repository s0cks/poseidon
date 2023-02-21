#include "heap/test_semispace.h"

#include "poseidon/flags.h"
#include "poseidon/object.h"

#include "matchers/is_pointer_to.h"
#include "mock_raw_object_visitor.h"
#include "assertions/ptr_assertions.h"
#include "assertions/type_assertions.h"

namespace poseidon {
 using namespace ::testing;

 TEST_F(SemispaceTest, TestVisitMarkedPointers_WillPass_VisitsNothing) {
   MockRawObjectVisitor visitor;
   ON_CALL(visitor, Visit(_))
       .WillByDefault(Return(false));
   ASSERT_TRUE(semispace().VisitMarkedPointers(&visitor));
 }

 TEST_F(SemispaceTest, TestVisitMarkedPointers_WillPass_VisitsOneNull) {
   MockRawObjectVisitor visitor;

   auto a = Null::TryAllocateIn(&semispace());
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsNull(a));
   ASSERT_FALSE(IsMarked(a));
   ASSERT_NO_FATAL_FAILURE(Mark(a));
   ASSERT_TRUE(IsMarked(a));
   EXPECT_CALL(visitor, Visit(IsPointerTo(a)));

   ASSERT_TRUE(semispace().VisitMarkedPointers(&visitor));
 }

 TEST_F(SemispaceTest, TestVisitMarkedPointers_WillPass_VisitsOneTrue) {
   MockRawObjectVisitor visitor;

   auto a = Bool::TryAllocateIn(&semispace(), true);
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsBool(a));
   ASSERT_TRUE(BoolEq(true, a));
   ASSERT_FALSE(IsMarked(a));
   ASSERT_NO_FATAL_FAILURE(Mark(a));
   ASSERT_TRUE(IsMarked(a));
   EXPECT_CALL(visitor, Visit(IsPointerTo(a)));

   ASSERT_TRUE(semispace().VisitMarkedPointers(&visitor));
 }

 TEST_F(SemispaceTest, TestVisitMarkedPointers_WillPass_VisitsOneFalse) {
   MockRawObjectVisitor visitor;

   auto a = Bool::TryAllocateIn(&semispace(), false);
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsBool(a));
   ASSERT_TRUE(BoolEq(false, a));
   ASSERT_FALSE(IsMarked(a));
   ASSERT_NO_FATAL_FAILURE(Mark(a));
   ASSERT_TRUE(IsMarked(a));
   EXPECT_CALL(visitor, Visit(IsPointerTo(a)));

   ASSERT_TRUE(semispace().VisitMarkedPointers(&visitor));
 }

#define DEFINE_VISIT_POINTERS_NUMBER_TYPE_PASSES_NEW_ZONE_TEST(Type) \
 TEST_F(SemispaceTest, TestVisitMarkedPointers_WillPass_VisitsOne##Type) { \
   MockRawObjectVisitor visitor;                                     \
   static constexpr const Raw##Type kAValue = 0xA;                   \
   auto a = Type::TryAllocateIn(&semispace(), kAValue);              \
   ASSERT_TRUE(IsAllocated(a));                                      \
   ASSERT_TRUE(Is##Type(a));                                         \
   ASSERT_TRUE(Type##Eq(kAValue, a));                                \
   ASSERT_FALSE(IsMarked(a));                                        \
   ASSERT_NO_FATAL_FAILURE(Mark(a));                                 \
   ASSERT_TRUE(IsMarked(a));                                         \
   EXPECT_CALL(visitor, Visit(IsPointerTo(a)));                      \
   ASSERT_TRUE(semispace().VisitMarkedPointers(&visitor));           \
 }

 FOR_EACH_INT_TYPE(DEFINE_VISIT_POINTERS_NUMBER_TYPE_PASSES_NEW_ZONE_TEST)
#undef DEFINE_VISIT_POINTERS_NUMBER_TYPE_PASSES_NEW_ZONE_TEST

 TEST_F(SemispaceTest, TestVisitMarkedPointers_WillPass_VisitsMultipleNulls) {
   MockRawObjectVisitor visitor;

   auto a = Null::TryAllocateIn(&semispace());
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsNull(a));
   ASSERT_FALSE(IsMarked(a));
   ASSERT_NO_FATAL_FAILURE(Mark(a));
   ASSERT_TRUE(IsMarked(a));
   EXPECT_CALL(visitor, Visit(IsPointerTo(a)));

   auto b = Null::TryAllocateIn(&semispace());
   ASSERT_TRUE(IsAllocated(b));
   ASSERT_TRUE(IsNull(b));
   ASSERT_FALSE(IsMarked(b));
   ASSERT_NO_FATAL_FAILURE(Mark(b));
   ASSERT_TRUE(IsMarked(b));
   EXPECT_CALL(visitor, Visit(IsPointerTo(b)));

   ASSERT_TRUE(semispace().VisitMarkedPointers(&visitor));
 }

 TEST_F(SemispaceTest, TestVisitMarkedPointers_WillPass_VisitsMultipleTrues) {
   MockRawObjectVisitor visitor;

   auto a = Bool::TryAllocateIn(&semispace(), true);
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsBool(a));
   ASSERT_TRUE(BoolEq(true, a));
   ASSERT_FALSE(IsMarked(a));
   ASSERT_NO_FATAL_FAILURE(Mark(a));
   ASSERT_TRUE(IsMarked(a));
   EXPECT_CALL(visitor, Visit(IsPointerTo(a)));

   auto b = Bool::TryAllocateIn(&semispace(), true);
   ASSERT_TRUE(IsAllocated(b));
   ASSERT_TRUE(IsBool(b));
   ASSERT_TRUE(BoolEq(true, b));
   ASSERT_FALSE(IsMarked(b));
   ASSERT_NO_FATAL_FAILURE(Mark(b));
   ASSERT_TRUE(IsMarked(b));
   EXPECT_CALL(visitor, Visit(IsPointerTo(b)));

   ASSERT_TRUE(semispace().VisitMarkedPointers(&visitor));
 }

 TEST_F(SemispaceTest, TestVisitMarkedPointers_WillPass_VisitsMultipleFalses) {
   MockRawObjectVisitor visitor;

   auto a = Bool::TryAllocateIn(&semispace(), false);
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsBool(a));
   ASSERT_TRUE(BoolEq(false, a));
   ASSERT_FALSE(IsMarked(a));
   ASSERT_NO_FATAL_FAILURE(Mark(a));
   ASSERT_TRUE(IsMarked(a));
   EXPECT_CALL(visitor, Visit(IsPointerTo(a)));

   auto b = Bool::TryAllocateIn(&semispace(), false);
   ASSERT_TRUE(IsAllocated(b));
   ASSERT_TRUE(IsBool(b));
   ASSERT_TRUE(BoolEq(false, b));
   ASSERT_FALSE(IsMarked(b));
   ASSERT_NO_FATAL_FAILURE(Mark(b));
   ASSERT_TRUE(IsMarked(b));
   EXPECT_CALL(visitor, Visit(IsPointerTo(b)));

   ASSERT_TRUE(semispace().VisitMarkedPointers(&visitor));
 }

#define DEFINE_VISIT_MULTIPLE_MARKED_POINTERS_NUMBER_TYPE_PASSES_SEMISPACE_TEST(Type) \
 TEST_F(SemispaceTest, TestVisitMarkedPointers_WillPass_VisitsMultiple##Type##s) {    \
   MockRawObjectVisitor visitor;                                                      \
   static constexpr const Raw##Type kAValue = 0xA;                                    \
   auto a = Type::TryAllocateIn(&semispace(), kAValue);                               \
   ASSERT_TRUE(IsAllocated(a));                                                       \
   ASSERT_TRUE(Is##Type(a));                                                          \
   ASSERT_TRUE(Type##Eq(kAValue, a));                                                 \
   ASSERT_FALSE(IsMarked(a));                                                         \
   Mark(a);                                                                           \
   ASSERT_TRUE(IsMarked(a));                                                          \
   EXPECT_CALL(visitor, Visit(IsPointerTo(a)));                                       \
   static constexpr const Raw##Type kBValue = 0xB;                                    \
   auto b = Type::TryAllocateIn(&semispace(), kBValue);                               \
   ASSERT_TRUE(IsAllocated(b));                                                       \
   ASSERT_TRUE(Is##Type(b));                                                          \
   ASSERT_TRUE(Type##Eq(kBValue, b));                                                 \
   ASSERT_FALSE(IsMarked(b));                                                         \
   Mark(b);                                                                           \
   ASSERT_TRUE(IsMarked(b));                                                          \
   EXPECT_CALL(visitor, Visit(IsPointerTo(b)));                                       \
   ASSERT_TRUE(semispace().VisitMarkedPointers(&visitor));                            \
 }

 FOR_EACH_INT_TYPE(DEFINE_VISIT_MULTIPLE_MARKED_POINTERS_NUMBER_TYPE_PASSES_SEMISPACE_TEST);
#undef DEFINE_VISIT_MULTIPLE_POINTERS_NUMBER_TYPE_PASSES_SEMISPACE_TEST
}