#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "poseidon/object.h"
#include "poseidon/zone/new_zone.h"

#include "helpers.h"
#include "assertions/ptr_assertions.h"
#include "assertions/type_assertions.h"

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

 class NewZoneTest : public Test {
  protected:
   MemoryRegion region_;
   NewZone zone_;

   NewZoneTest():
    Test(),
    region_(flags::GetNewZoneSize(), MemoryRegion::kReadWrite),
    zone_(region_) {
   }

   inline MemoryRegion& region() {
     return region_;
   }

   inline NewZone& zone() {
     return zone_;
   }
  public:
   ~NewZoneTest() override = default;

   void SetUp() override {
     ASSERT_NO_FATAL_FAILURE(zone().SetWritable());
     ASSERT_NO_FATAL_FAILURE(zone().Clear());
     ASSERT_NO_FATAL_FAILURE(zone().SetReadOnly());
     ASSERT_NO_FATAL_FAILURE(NewZonePrinter::Print(&zone()));
     ASSERT_NO_FATAL_FAILURE(zone().SetWritable());
   }

   void TearDown() override {
     ASSERT_NO_FATAL_FAILURE(zone().SetReadOnly());
     ASSERT_NO_FATAL_FAILURE(NewZonePrinter::Print(&zone()));
   }
 };

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

#define DEFINE_TRY_ALLOCATE_BYTES_FAILS_NEW_ZONE_TEST(TestName, NumberOfBytes) \
 DEFINE_TRY_ALLOCATE_BYTES_FAILS_TEST(NewZoneTest, TestName, NewZone, flags::GetNewZoneSize(), NumberOfBytes)

 DEFINE_TRY_ALLOCATE_BYTES_FAILS_NEW_ZONE_TEST(SizeLessThanZero, -1);
 DEFINE_TRY_ALLOCATE_BYTES_FAILS_NEW_ZONE_TEST(SizeEqualsZero, 0);
 DEFINE_TRY_ALLOCATE_BYTES_FAILS_NEW_ZONE_TEST(SizeLessThanMin, NewZone::GetMinimumObjectSize() - 1);
 DEFINE_TRY_ALLOCATE_BYTES_FAILS_NEW_ZONE_TEST(SizeEqualsZoneSize, flags::GetNewZoneSize());
 DEFINE_TRY_ALLOCATE_BYTES_FAILS_NEW_ZONE_TEST(SizeGreaterThanZoneSize, flags::GetNewZoneSize() + 1);
 DEFINE_TRY_ALLOCATE_BYTES_FAILS_NEW_ZONE_TEST(SizeGreaterThanMax, NewZone::GetMaximumObjectSize() + 1);

#define DEFINE_TRY_ALLOCATE_BYTES_PASS_NEW_ZONE_TEST(TestName, NumberOfBytes) \
 DEFINE_TRY_ALLOCATE_BYTES_PASS_TEST(NewZoneTest, TestName, NewZone, flags::GetNewZoneSize(), NumberOfBytes)

 DEFINE_TRY_ALLOCATE_BYTES_PASS_NEW_ZONE_TEST(SizeEqualsMin, NewZone::GetMinimumObjectSize());
 DEFINE_TRY_ALLOCATE_BYTES_PASS_NEW_ZONE_TEST(SizeEqualsMax, NewZone::GetMaximumObjectSize());

 TEST_F(NewZoneTest, TestTryAllocate_Null_WillPass) {
   auto ptr = Null::TryAllocateIn(&zone());
   ASSERT_TRUE(IsAllocated(ptr));
   ASSERT_TRUE(IsNull(ptr));
   ASSERT_TRUE(IsNew(ptr));
   ASSERT_FALSE(IsOld(ptr));
   ASSERT_FALSE(IsFree(ptr));
   ASSERT_FALSE(IsMarked(ptr));
   ASSERT_FALSE(IsRemembered(ptr));
   ASSERT_FALSE(IsForwarding(ptr));
 }

 TEST_F(NewZoneTest, TestTryAllocate_True_WillPass) {
   auto ptr = Bool::TryAllocateIn(&zone(), true);
   ASSERT_TRUE(IsAllocated(ptr));
   ASSERT_TRUE(IsBool(ptr));
   ASSERT_TRUE(ptr->Get());
   ASSERT_TRUE(IsNew(ptr));
   ASSERT_FALSE(IsOld(ptr));
   ASSERT_FALSE(IsFree(ptr));
   ASSERT_FALSE(IsMarked(ptr));
   ASSERT_FALSE(IsRemembered(ptr));
   ASSERT_FALSE(IsForwarding(ptr));
 }

 TEST_F(NewZoneTest, TestTryAllocate_False_WillPass) {
   auto ptr = Bool::TryAllocateIn(&zone(), false);
   ASSERT_TRUE(IsAllocated(ptr));
   ASSERT_TRUE(IsBool(ptr));
   ASSERT_FALSE(ptr->Get());
   ASSERT_TRUE(IsNew(ptr));
   ASSERT_FALSE(IsOld(ptr));
   ASSERT_FALSE(IsFree(ptr));
   ASSERT_FALSE(IsMarked(ptr));
   ASSERT_FALSE(IsRemembered(ptr));
   ASSERT_FALSE(IsForwarding(ptr));
 }

 TEST_F(NewZoneTest, TestTryAllocate_Tuple_WillPass) {
   auto ptr = Tuple::TryAllocateIn(&zone());
   ASSERT_TRUE(IsAllocated(ptr));
   ASSERT_TRUE(IsTuple(ptr));
   ASSERT_TRUE(IsUnallocated(ptr->GetCarPointer()));
   ASSERT_TRUE(IsUnallocated(ptr->GetCdrPointer()));
   ASSERT_TRUE(IsNew(ptr));
   ASSERT_FALSE(IsOld(ptr));
   ASSERT_FALSE(IsFree(ptr));
   ASSERT_FALSE(IsMarked(ptr));
   ASSERT_FALSE(IsRemembered(ptr));
   ASSERT_FALSE(IsForwarding(ptr));
 }

#define DEFINE_TRY_ALLOCATE_NUMBER_TYPE_PASSES_NEW_ZONE_TEST(Type) \
 TEST_F(NewZoneTest, TestTryAllocate_##Type##_WillPass) {          \
   static constexpr const Raw##Type kValue = 0xA;                  \
   auto ptr = Type::TryAllocateIn(&zone(), kValue);                \
   ASSERT_TRUE(IsAllocated(ptr));                                  \
   ASSERT_TRUE(Is##Type(ptr));                                     \
   ASSERT_TRUE(Type##Eq(kValue, ptr));                             \
   ASSERT_TRUE(IsNew(ptr));                                        \
   ASSERT_FALSE(IsOld(ptr));                                       \
   ASSERT_FALSE(IsFree(ptr));                                      \
   ASSERT_FALSE(IsMarked(ptr));                                    \
   ASSERT_FALSE(IsRemembered(ptr));                                \
   ASSERT_FALSE(IsForwarding(ptr));                                \
   Semispace fromspace = zone().GetFromspace();                    \
   Semispace tospace = zone().GetTospace();                        \
   ASSERT_TRUE(fromspace.Contains((const Region&)*ptr->raw_ptr()));\
   ASSERT_FALSE(tospace.Contains((const Region&)*ptr->raw_ptr())); \
 }

 FOR_EACH_INT_TYPE(DEFINE_TRY_ALLOCATE_NUMBER_TYPE_PASSES_NEW_ZONE_TEST);

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
}