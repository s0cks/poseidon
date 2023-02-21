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

   auto fromspace = zone().GetFromspace();
   ASSERT_EQ(zone().GetStartingAddress(), fromspace.GetStartingAddress());
   ASSERT_EQ(zone().GetStartingAddress() + zone().GetSemispaceSize(), fromspace.GetEndingAddress());

   auto tospace = zone().GetTospace();
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

     auto fromspace = zone().GetFromspace();
     ASSERT_EQ(zone().GetStartingAddress(), fromspace.GetStartingAddress());
     ASSERT_EQ(zone().GetStartingAddress() + zone().GetSemispaceSize(), fromspace.GetEndingAddress());

     auto tospace = zone().GetTospace();
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

     auto fromspace = zone().GetFromspace();
     ASSERT_EQ(zone().GetStartingAddress(), fromspace.GetStartingAddress());
     ASSERT_EQ(zone().GetStartingAddress() + zone().GetSemispaceSize(), fromspace.GetEndingAddress());

     auto tospace = zone().GetTospace();
     ASSERT_EQ(zone().GetStartingAddress() + zone().GetSemispaceSize(), tospace.GetStartingAddress());
     ASSERT_EQ(zone().GetEndingAddress(), tospace.GetEndingAddress());

     ASSERT_FALSE(fromspace.Contains(tospace));
     ASSERT_TRUE(fromspace.Intersects(tospace));

     ASSERT_FALSE(tospace.Contains(fromspace));
     ASSERT_TRUE(tospace.Intersects(fromspace));
   }
 }
}