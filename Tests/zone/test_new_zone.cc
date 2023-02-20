#include "zone/test_new_zone.h"

#include "poseidon/object.h"
#include "poseidon/marker/marker.h"

#include "helpers.h"
#include "assertions/ptr_assertions.h"
#include "assertions/type_assertions.h"

#include "matchers/is_pointer_to.h"
#include "mock_raw_object_visitor.h"

namespace poseidon{
 using namespace ::testing;

 class IsNewPageMatcher {
  protected:
   const NewPage* expected_;
  public:
   using is_gtest_matcher = void;

   explicit IsNewPageMatcher(const NewPage* expected):
       expected_(expected) {
   }

   bool MatchAndExplain(NewPage* page, std::ostream*) const {
     return (*expected_) == (*page);
   }

   void DescribeTo(std::ostream* stream) const {
     (*stream) << "NewPage equals " << (*expected_);
   }

   void DescribeNegationTo(std::ostream* stream) const {
     (*stream) << "NewPage does not equal " << (*expected_);
   }
 };

 Matcher<NewPage*> NewPageEq(const NewPage* expected) {
   return IsNewPageMatcher(expected);
 }

 TEST_F(NewZoneTest, TestConstructor_WillPass) {
   ASSERT_EQ((const Region&)region(), (const Region&)zone());
   ASSERT_EQ(zone().GetSize() / 2, zone().GetSemispaceSize());
   ASSERT_TRUE(zone().IsEmpty());

   Semispace fromspace = zone().GetFromspace();
   ASSERT_EQ(zone().GetStartingAddress(), fromspace.GetStartingAddress());
   ASSERT_EQ(zone().GetStartingAddress() + zone().GetSemispaceSize(), fromspace.GetEndingAddress());

   Semispace tospace = zone().GetTospace();
   ASSERT_EQ(zone().GetStartingAddress() + zone().GetSemispaceSize(), tospace.GetStartingAddress());
   ASSERT_EQ(zone().GetEndingAddress(), tospace.GetEndingAddress());

   ASSERT_FALSE(fromspace.Contains(tospace));
   ASSERT_TRUE(fromspace.Intersects(tospace));

   ASSERT_FALSE(tospace.Contains(fromspace));
   ASSERT_TRUE(tospace.Intersects(fromspace));

   //TODO: check pages
 }

 TEST_F(NewZoneTest, TestGetSemispaceSize) {
   auto expected = zone().GetSize() / 2;
   ASSERT_EQ(expected, zone().GetSemispaceSize());
 }

 TEST_F(NewZoneTest, TestGetFromspaceStartingAddress) {
   auto expected = zone().GetStartingAddress();
   ASSERT_EQ(expected, zone().GetFromspaceStartingAddress());
 }

 TEST_F(NewZoneTest, TestGetFromspaceEndingAddress) {
   auto expected = zone().GetStartingAddress() + zone().GetSemispaceSize();
   ASSERT_EQ(expected, zone().GetFromspaceEndingAddress());
 }

 TEST_F(NewZoneTest, TestGetFromspace) {
   Semispace semispace = zone().GetFromspace();
   ASSERT_TRUE(semispace.IsFromspace());
   ASSERT_TRUE(semispace.IsEmpty());
   ASSERT_EQ(Region(zone().GetStartingAddress(), zone().GetSemispaceSize()), semispace);
 }

 TEST_F(NewZoneTest, TestGetTospaceStartingAddress) {
   auto expected = zone().GetStartingAddress() + zone().GetSemispaceSize();
   ASSERT_EQ(expected, zone().GetTospaceStartingAddress());
 }

 TEST_F(NewZoneTest, TestGetTospaceEndingAddress) {
   auto expected = zone().GetEndingAddress();
   ASSERT_EQ(expected, zone().GetTospaceEndingAddress());
 }

 TEST_F(NewZoneTest, TestSwapSpaces_WillPass) {
//   MemoryRegion region(flags::GetNewZoneSize());
//   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
//   auto zone = NewZone::New(region);
//   ASSERT_NO_FATAL_FAILURE(zone->SetWritable());
//
//   Semispace fromspace = zone->fromspace();
//   ASSERT_EQ(fromspace, zone->fromspace());
//   Semispace tospace = zone->tospace();
//   ASSERT_EQ(tospace, zone->tospace());
//   ASSERT_NE(fromspace, tospace);
//   ASSERT_NO_FATAL_FAILURE(SwapSpaces(zone));
//   ASSERT_NE(fromspace, tospace);
//   ASSERT_EQ(fromspace, zone->tospace());
//   ASSERT_EQ(tospace, zone->fromspace());
 }

 TEST_F(NewZoneTest, TestIsEmpty) {
   ASSERT_TRUE(zone().IsEmpty());

   static constexpr const RawInt32 kAValue = 0xA;
   auto a = Int32::TryAllocateIn(&zone(), kAValue);
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsInt32(a));
   ASSERT_TRUE(Int32Eq(kAValue, a));

   ASSERT_FALSE(zone().IsEmpty());
 }

 TEST_F(NewZoneTest, TestClear_WillPass){
   {
     ASSERT_EQ((const Region&) region(), (const Region&) zone());
     ASSERT_EQ(zone().GetSize() / 2, zone().GetSemispaceSize());
     ASSERT_TRUE(zone().IsEmpty());

     Semispace fromspace = zone().GetFromspace();
     ASSERT_EQ(zone().GetStartingAddress(), fromspace.GetStartingAddress());
     ASSERT_EQ(zone().GetStartingAddress() + zone().GetSemispaceSize(), fromspace.GetEndingAddress());

     Semispace tospace = zone().GetTospace();
     ASSERT_EQ(zone().GetStartingAddress() + zone().GetSemispaceSize(), tospace.GetStartingAddress());
     ASSERT_EQ(zone().GetEndingAddress(), tospace.GetEndingAddress());

     ASSERT_FALSE(fromspace.Contains(tospace));
     ASSERT_TRUE(fromspace.Intersects(tospace));

     ASSERT_FALSE(tospace.Contains(fromspace));
     ASSERT_TRUE(tospace.Intersects(fromspace));
   }

   auto ptr = Null::TryAllocateIn(&zone());
   ASSERT_TRUE(IsAllocated(ptr));
   ASSERT_TRUE(IsNull(ptr));
   ASSERT_TRUE(IsNew(ptr));
   ASSERT_FALSE(IsOld(ptr));
   ASSERT_FALSE(IsFree(ptr));
   ASSERT_FALSE(IsMarked(ptr));
   ASSERT_FALSE(IsRemembered(ptr));
   ASSERT_FALSE(IsForwarding(ptr));
   ASSERT_FALSE(zone().IsEmpty());
   ASSERT_NO_FATAL_FAILURE(zone().Clear());

   {
     ASSERT_EQ((const Region&) region(), (const Region&) zone());
     ASSERT_EQ(zone().GetSize() / 2, zone().GetSemispaceSize());

     Semispace fromspace = zone().GetFromspace();
     ASSERT_EQ(zone().GetStartingAddress(), fromspace.GetStartingAddress());
     ASSERT_EQ(zone().GetStartingAddress() + zone().GetSemispaceSize(), fromspace.GetEndingAddress());

     Semispace tospace = zone().GetTospace();
     ASSERT_EQ(zone().GetStartingAddress() + zone().GetSemispaceSize(), tospace.GetStartingAddress());
     ASSERT_EQ(zone().GetEndingAddress(), tospace.GetEndingAddress());

     ASSERT_FALSE(fromspace.Contains(tospace));
     ASSERT_TRUE(fromspace.Intersects(tospace));

     ASSERT_FALSE(tospace.Contains(fromspace));
     ASSERT_TRUE(tospace.Intersects(fromspace));
   }
 }

 TEST_F(NewZoneTest, TestVisitPointers_WillPass_VisitsNothing) {
   MockRawObjectVisitor visitor;
   ON_CALL(visitor, Visit(_))
    .WillByDefault(Return(false));
   ASSERT_TRUE(zone().VisitPointers(&visitor));
 }

 TEST_F(NewZoneTest, TestVisitPointers_WillPass_VisitsOneNull) {
   MockRawObjectVisitor visitor;

   auto a = Null::TryAllocateIn(&zone());
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsNull(a));
   EXPECT_CALL(visitor, Visit(IsPointerTo(a)));

   ASSERT_NO_FATAL_FAILURE(zone().VisitPointers(&visitor));
 }

 TEST_F(NewZoneTest, TestVisitPointers_WillPass_VisitsOneTrue) {
   MockRawObjectVisitor visitor;

   auto a = Bool::TryAllocateIn(&zone(), true);
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsBool(a));
   ASSERT_TRUE(BoolEq(true, a));
   EXPECT_CALL(visitor, Visit(IsPointerTo(a)));

   ASSERT_NO_FATAL_FAILURE(zone().VisitPointers(&visitor));
 }

 TEST_F(NewZoneTest, TestVisitPointers_WillPass_VisitsOneFalse) {
   MockRawObjectVisitor visitor;

   auto a = Bool::TryAllocateIn(&zone(), false);
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsBool(a));
   ASSERT_TRUE(BoolEq(false, a));
   EXPECT_CALL(visitor, Visit(IsPointerTo(a)));

   ASSERT_NO_FATAL_FAILURE(zone().VisitPointers(&visitor));
 }

#define DEFINE_VISIT_POINTERS_NUMBER_TYPE_PASSES_NEW_ZONE_TEST(Type) \
 TEST_F(NewZoneTest, TestVisitPointers_WillPass_VisitsOne##Type) {   \
   MockRawObjectVisitor visitor;                                     \
   static constexpr const Raw##Type kAValue = 0xA;                   \
   auto a = Type::TryAllocateIn(&zone(), kAValue);                   \
   ASSERT_TRUE(IsAllocated(a));                                      \
   ASSERT_TRUE(Is##Type(a));                                         \
   ASSERT_TRUE(Type##Eq(kAValue, a));                                \
   EXPECT_CALL(visitor, Visit(IsPointerTo(a)));                      \
   ASSERT_NO_FATAL_FAILURE(zone().VisitPointers(&visitor));          \
 }
 FOR_EACH_INT_TYPE(DEFINE_VISIT_POINTERS_NUMBER_TYPE_PASSES_NEW_ZONE_TEST);
#undef DEFINE_VISIT_POINTERS_NUMBER_TYPE_PASSES_NEW_ZONE_TEST

 TEST_F(NewZoneTest, TestVisitPointers_WillPass_VisitsMultipleNulls) {
   MockRawObjectVisitor visitor;

   auto a = Null::TryAllocateIn(&zone());
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsNull(a));
   EXPECT_CALL(visitor, Visit(IsPointerTo(a)));

   auto b = Null::TryAllocateIn(&zone());
   ASSERT_TRUE(IsAllocated(b));
   ASSERT_TRUE(IsNull(b));
   EXPECT_CALL(visitor, Visit(IsPointerTo(b)));

   ASSERT_NO_FATAL_FAILURE(zone().VisitPointers(&visitor));
 }

 TEST_F(NewZoneTest, TestVisitPointers_WillPass_VisitsMultipleTrues) {
   MockRawObjectVisitor visitor;

   auto a = Bool::TryAllocateIn(&zone(), true);
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsBool(a));
   ASSERT_TRUE(BoolEq(true, a));
   EXPECT_CALL(visitor, Visit(IsPointerTo(a)));

   auto b = Bool::TryAllocateIn(&zone(), true);
   ASSERT_TRUE(IsAllocated(b));
   ASSERT_TRUE(IsBool(b));
   ASSERT_TRUE(BoolEq(true, b));
   EXPECT_CALL(visitor, Visit(IsPointerTo(b)));

   ASSERT_NO_FATAL_FAILURE(zone().VisitPointers(&visitor));
 }

 TEST_F(NewZoneTest, TestVisitPointers_WillPass_VisitsMultipleFalses) {
   MockRawObjectVisitor visitor;

   auto a = Bool::TryAllocateIn(&zone(), false);
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsBool(a));
   ASSERT_TRUE(BoolEq(false, a));
   EXPECT_CALL(visitor, Visit(IsPointerTo(a)));

   auto b = Bool::TryAllocateIn(&zone(), false);
   ASSERT_TRUE(IsAllocated(b));
   ASSERT_TRUE(IsBool(b));
   ASSERT_TRUE(BoolEq(false, b));
   EXPECT_CALL(visitor, Visit(IsPointerTo(b)));

   ASSERT_NO_FATAL_FAILURE(zone().VisitPointers(&visitor));
 }

#define DEFINE_VISIT_MULTIPLE_POINTERS_NUMBER_TYPE_PASSES_NEW_ZONE_TEST(Type) \
 TEST_F(NewZoneTest, TestVisitPointers_WillPass_VisitsMultiple##Type##s) {    \
   MockRawObjectVisitor visitor;                                              \
   static constexpr const Raw##Type kAValue = 0xA;                            \
   auto a = Type::TryAllocateIn(&zone(), kAValue);                            \
   ASSERT_TRUE(IsAllocated(a));                                               \
   ASSERT_TRUE(Is##Type(a));                                                  \
   ASSERT_TRUE(Type##Eq(kAValue, a));                                         \
   EXPECT_CALL(visitor, Visit(IsPointerTo(a)));                               \
   static constexpr const Raw##Type kBValue = 0xB;                            \
   auto b = Type::TryAllocateIn(&zone(), kBValue);                            \
   ASSERT_TRUE(IsAllocated(b));                                               \
   ASSERT_TRUE(Is##Type(b));                                                  \
   ASSERT_TRUE(Type##Eq(kBValue, b));                                         \
   EXPECT_CALL(visitor, Visit(IsPointerTo(b)));                               \
   ASSERT_NO_FATAL_FAILURE(zone().VisitPointers(&visitor));                   \
 }
 FOR_EACH_INT_TYPE(DEFINE_VISIT_MULTIPLE_POINTERS_NUMBER_TYPE_PASSES_NEW_ZONE_TEST)
#undef DEFINE_VISIT_MULTIPLE_POINTERS_NUMBER_TYPE_PASSES_NEW_ZONE_TEST

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