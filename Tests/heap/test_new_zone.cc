#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "helpers.h"
#include "poseidon/heap/new_zone.h"
#include "heap/mock_new_page_visitor.h"
#include "assertions/type_assertions.h"
#include "mock_raw_object_visitor.h"

#include "matchers/is_pointer_to.h"
#include "helpers/type_alloc_helpers.h"

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
   NewZoneTest() = default;

   static inline void
   SwapSpaces(NewZone* zone) {
     zone->SwapSpaces();
   }

   static inline uword
   TryAllocateBytes(NewZone* zone, const ObjectSize size) {
     return zone->TryAllocate(size);
   }

   static inline Pointer*
   TryAllocateWord(NewZone* zone, word value) {
     auto address = TryAllocateBytes(zone, kWordSize);
     if (address == UNALLOCATED)
       return nullptr;
     auto ptr = (Pointer*)address;
     (*((word*)ptr->GetObjectPointerAddress())) = value;
     return ptr;
   }

   static inline Pointer*
   TryAllocateMarkedWord(NewZone* zone, word value) {
     auto address = TryAllocateBytes(zone, kWordSize);
     if (address == UNALLOCATED)
       return nullptr;
     auto ptr = (Pointer*)address;
     ptr->SetMarkedBit();
     (*((word*)ptr->GetObjectPointerAddress())) = value;
     return ptr;
   }
  public:
   ~NewZoneTest() override = default;
 };

 TEST_F(NewZoneTest, TestConstructor) {
   static const int64_t kSemispaceSize = GetNewZoneSize() / 2;
   static const int64_t kFromspaceOffset = 0;
   static const int64_t kTospaceOffset = kSemispaceSize;

   MemoryRegion region(GetNewZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto zone = NewZone::New(region);
   ASSERT_EQ(zone->GetStartingAddress(), region.GetStartingAddress());
   ASSERT_EQ(zone->GetSize(), region.GetSize());
   ASSERT_EQ(zone->semisize(), kSemispaceSize);

   Semispace& fromspace = zone->fromspace();
   ASSERT_EQ(fromspace.GetStartingAddress(), region.GetStartingAddress() + kFromspaceOffset);
   ASSERT_EQ(fromspace.GetSize(), kSemispaceSize);
   ASSERT_TRUE(fromspace.IsEmpty());

   Semispace& tospace = zone->tospace();
   ASSERT_EQ(tospace.GetStartingAddress(), region.GetStartingAddress() + kTospaceOffset);
   ASSERT_EQ(tospace.GetSize(), kSemispaceSize);
   ASSERT_TRUE(tospace.IsEmpty());

   for(auto idx = 0; idx < GetNumberOfNewPages(); idx++) {
     ASSERT_FALSE(zone->IsMarked(idx));
     auto page = zone->pages(idx);
     ASSERT_EQ(page->GetIndex(), idx);
     ASSERT_EQ(page->GetStartingAddress(), zone->GetStartingAddress() + (idx * GetNewPageSize()));
     ASSERT_EQ(page->GetSize(), GetNewPageSize());
   }
 }

 //TODO: add equals & not equals tests?

 TEST_F(NewZoneTest, TestSwapSpaces_WillPass) {
   MemoryRegion region(GetNewZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto zone = NewZone::New(region);
   ASSERT_NO_FATAL_FAILURE(zone->SetWritable());

   Semispace fromspace = zone->fromspace();
   ASSERT_EQ(fromspace, zone->fromspace());
   Semispace tospace = zone->tospace();
   ASSERT_EQ(tospace, zone->tospace());
   ASSERT_NE(fromspace, tospace);
   ASSERT_NO_FATAL_FAILURE(SwapSpaces(zone));
   ASSERT_NE(fromspace, tospace);
   ASSERT_EQ(fromspace, zone->tospace());
   ASSERT_EQ(tospace, zone->fromspace());
 }

#define DEFINE_TRY_ALLOCATE_BYTES_FAILS_NEW_ZONE_TEST(TestName, NumberOfBytes) \
 DEFINE_TRY_ALLOCATE_BYTES_FAILS_TEST(NewZoneTest, TestName, NewZone, GetNewZoneSize(), NumberOfBytes)

 DEFINE_TRY_ALLOCATE_BYTES_FAILS_NEW_ZONE_TEST(SizeLessThanZero, -1);
 DEFINE_TRY_ALLOCATE_BYTES_FAILS_NEW_ZONE_TEST(SizeEqualsZero, 0);
 DEFINE_TRY_ALLOCATE_BYTES_FAILS_NEW_ZONE_TEST(SizeLessThanMin, NewZone::GetMinimumObjectSize() - 1);
 DEFINE_TRY_ALLOCATE_BYTES_FAILS_NEW_ZONE_TEST(SizeEqualsZoneSize, GetNewZoneSize());
 DEFINE_TRY_ALLOCATE_BYTES_FAILS_NEW_ZONE_TEST(SizeGreaterThanZoneSize, GetNewZoneSize() + 1);
 DEFINE_TRY_ALLOCATE_BYTES_FAILS_NEW_ZONE_TEST(SizeGreaterThanMax, NewZone::GetMaximumObjectSize() + 1);

#define DEFINE_TRY_ALLOCATE_BYTES_PASS_NEW_ZONE_TEST(TestName, NumberOfBytes) \
 DEFINE_TRY_ALLOCATE_BYTES_PASS_TEST(NewZoneTest, TestName, NewZone, GetNewZoneSize(), NumberOfBytes)

 DEFINE_TRY_ALLOCATE_BYTES_PASS_NEW_ZONE_TEST(SizeEqualsMin, NewZone::GetMinimumObjectSize());
 DEFINE_TRY_ALLOCATE_BYTES_PASS_NEW_ZONE_TEST(SizeEqualsMax, NewZone::GetMaximumObjectSize());
 DEFINE_TRY_ALLOCATE_BYTES_PASS_NEW_ZONE_TEST(SizeEqualsWordSize, kWordSize);

 TEST_F(NewZoneTest, TestTryAllocateBytes_WillPass){
   NewZone zone(GetNewZoneSize());
   ASSERT_NO_FATAL_FAILURE(zone.SetWritable());

   Semispace& fromspace = zone.fromspace();

   static const constexpr RawLong kDefaultWordValue = 42;
   auto ptr = Long::TryAllocateIn(&zone, kDefaultWordValue);
   ASSERT_NE(ptr, nullptr);
   ASSERT_TRUE(IsLong(ptr->raw_ptr()));
   ASSERT_TRUE(LongEq(kDefaultWordValue, ptr));

   // the object should be inside the fromspace
   ASSERT_TRUE(fromspace.Intersects(*ptr->raw_ptr()));

   // the object should be in the first page, and the page should be marked
   static const int64_t kFirstPageIndex = 0;
   ASSERT_TRUE(zone.IsMarked(kFirstPageIndex));
   ASSERT_TRUE(zone.pages(kFirstPageIndex)->Intersects(*ptr->raw_ptr()));
 }

 TEST_F(NewZoneTest, TestVisitPages) {
   MemoryRegion region(GetNewZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto zone = NewZone::New(region);

   MockNewPageVisitor visitor;
   EXPECT_CALL(visitor, Visit(_))
    .Times(static_cast<int>(GetNumberOfNewPages()))
    .WillRepeatedly(Return(true));
   ASSERT_NO_FATAL_FAILURE(zone->VisitPages(&visitor));
 }

 TEST_F(NewZoneTest, TestVisitMarkedPages) {
   MemoryRegion region(GetNewZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto zone = NewZone::New(region);

   auto p1 = zone->pages(1);
   ASSERT_NO_FATAL_FAILURE(zone->Mark(p1));
   auto p2 = zone->pages(3);
   ASSERT_NO_FATAL_FAILURE(zone->Mark(p2));
   auto p3 = zone->pages(5);
   ASSERT_NO_FATAL_FAILURE(zone->Mark(p3));

   MockNewPageVisitor visitor;
   EXPECT_CALL(visitor, Visit(NewPageEq(p1)))
    .Times(1)
    .WillOnce(Return(true));
   EXPECT_CALL(visitor, Visit(NewPageEq(p2)))
    .Times(1)
    .WillOnce(Return(true));
   EXPECT_CALL(visitor, Visit(NewPageEq(p3)))
    .Times(1)
    .WillOnce(Return(true));
   ASSERT_NO_FATAL_FAILURE(zone->VisitMarkedPages(&visitor));
 }

 TEST_F(NewZoneTest, TestVisitPointers_WillPass_ContiguousUnmarked) {
   NewZone zone(GetNewZoneSize());
   ASSERT_NO_FATAL_FAILURE(zone.SetWritable());

   MockRawObjectVisitor visitor;
   static constexpr const int64_t kNumberOfPointers = 4;
   GenerateUnmarkedLongsInZone(&zone, kNumberOfPointers);
   EXPECT_CALL(visitor, Visit(_))
    .Times(kNumberOfPointers);
   ASSERT_TRUE(zone.VisitPointers(&visitor));
 }

 TEST_F(NewZoneTest, TestVisitPointers_WillPass_ContiguousMarkedAndUnmarked) {
   NewZone zone(GetNewZoneSize());
   ASSERT_NO_FATAL_FAILURE(zone.SetWritable());

   MockRawObjectVisitor visitor;
   static constexpr const int64_t kNumberOfUnmarkedPointers = 4;
   GenerateUnmarkedLongsInZone(&zone, kNumberOfUnmarkedPointers);
   static constexpr const int64_t kNumberOfMarkedPointers = 3;
   GenerateMarkedLongsInZone(&zone, kNumberOfMarkedPointers);

   EXPECT_CALL(visitor, Visit(_))
     .Times(kNumberOfUnmarkedPointers + kNumberOfMarkedPointers);
   ASSERT_TRUE(zone.VisitPointers(&visitor));
 }

 TEST_F(NewZoneTest, TestVisitMarkedPointers) {
   NewZone zone(GetNewZoneSize());
   ASSERT_NO_FATAL_FAILURE(zone.SetWritable());

   MockRawObjectVisitor visitor;
   static const constexpr int64_t kNumberOfPointers = 4;
   GenerateUnmarkedLongsInZone(&zone, kNumberOfPointers);

   static constexpr const int64_t kNumberOfMarkedPointers = 3;
   GenerateMarkedLongsInZone(&zone, kNumberOfMarkedPointers);

   EXPECT_CALL(visitor, Visit(_))
    .Times(kNumberOfMarkedPointers);
   ASSERT_NO_FATAL_FAILURE(zone.VisitMarkedPointers(&visitor));
 }
}