#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "helpers.h"
#include "poseidon/heap/new_zone.h"
#include "heap/mock_new_page_visitor.h"

namespace poseidon{
#define UNALLOCATED 0 //TODO: cleanup

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

   static inline RawObject*
   TryAllocateWord(NewZone* zone, word value) {
     auto address = TryAllocateBytes(zone, kWordSize);
     if (address == UNALLOCATED)
       return nullptr;
     auto ptr = (RawObject*)address;
     (*((word*)ptr->GetObjectPointerAddress())) = value;
     return ptr;
   }

   static inline RawObject*
   TryAllocateMarkedWord(NewZone* zone, word value) {
     auto address = TryAllocateBytes(zone, kWordSize);
     if (address == UNALLOCATED)
       return nullptr;
     auto ptr = (RawObject*)address;
     ptr->SetMarkedBit();
     (*((word*)ptr->GetObjectPointerAddress())) = value;
     return ptr;
   }
  public:
   ~NewZoneTest() override = default;
 };

 static inline const int64_t
 CalculateNewZoneSize() {
   auto size = GetNewZoneSize();
   size += NewZone::GetHeaderSize();
   size += (GetNumberOfNewPages() * GetNewPageSize());
   return size;
 }

 TEST_F(NewZoneTest, TestConstructor) {
   static const int64_t kHeaderSize = NewZone::GetHeaderSize();
   static const int64_t kZoneSize = GetNewZoneSize();
   static const int64_t kTotalZoneSize = kHeaderSize + kZoneSize;
   static const int64_t kSemispaceSize = kZoneSize / 2;

   static const int64_t kZoneOffset = kHeaderSize;
   static const int64_t kFromspaceOffset = kZoneOffset;
   static const int64_t kTospaceOffset = kZoneOffset + kSemispaceSize;

   MemoryRegion region(kTotalZoneSize);
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto zone = NewZone::New(region);
   ASSERT_EQ(zone->GetZoneStartingAddress(), region.GetStartingAddress());
   ASSERT_EQ(zone->GetStartingAddress(), region.GetStartingAddress() + kZoneOffset);
   ASSERT_EQ(zone->GetTotalSize(), kTotalZoneSize);
   ASSERT_EQ(zone->GetSize(), kZoneSize);
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
     ASSERT_FALSE(zone->IsPageMarked(idx));
     auto page = zone->GetPageAt(idx);
     ASSERT_EQ(page->GetStartingAddress(), zone->GetStartingAddress() + (idx * GetNewPageSize()));
     ASSERT_EQ(page->GetSize(), GetNewPageSize());
   }
 }

 //TODO: add equals & not equals tests?

 TEST_F(NewZoneTest, TestSwapSpaces_WillPass) {
   MemoryRegion region(GetNewZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto zone = NewZone::New(region);

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

 TEST_F(NewZoneTest, TestTryAllocate_WillFail_SizeLessThanZero) {
   MemoryRegion region(GetNewZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto zone = NewZone::New(region);
   auto ptr = TryAllocateBytes(zone, -1);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(NewZoneTest, TestTryAllocate_WillFail_SizeEqualToZero) {
   MemoryRegion region(GetNewZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto zone = NewZone::New(region);
   auto ptr = TryAllocateBytes(zone, 0);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(NewZoneTest, TestTryAllocate_WillFail_SizeEqualToPageSize) {
   MemoryRegion region(GetNewZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto zone = NewZone::New(region);
   auto ptr = TryAllocateBytes(zone, GetNewPageSize());
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(NewZoneTest, TestTryAllocate_WillFail_SizeEqualToZoneSize) {
   MemoryRegion region(GetNewZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto zone = NewZone::New(region);
   auto ptr = TryAllocateBytes(zone, GetNewZoneSize());
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(NewZoneTest, TestTryAllocate_WillFail_SizeGreaterThanZoneSize) {
   MemoryRegion region(GetNewZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto zone = NewZone::New(region);
   auto ptr = TryAllocateBytes(zone, GetNewZoneSize() + 1);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(NewZoneTest, TestTryAllocate_WillPass){
   MemoryRegion region(GetNewZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto zone = NewZone::New(region);
   Semispace& fromspace = zone->fromspace();

   static const constexpr word kDefaultWordValue = 42;
   auto ptr = TryAllocateWord(zone, kDefaultWordValue);
   ASSERT_TRUE(IsAllocated(ptr));
   ASSERT_TRUE(IsNew(ptr));
   ASSERT_FALSE(IsOld(ptr));
   ASSERT_FALSE(IsMarked(ptr));
   ASSERT_FALSE(IsRemembered(ptr));
   ASSERT_FALSE(IsForwarding(ptr));
   ASSERT_TRUE(IsWord(ptr, kDefaultWordValue));

   // the object should be inside the fromspace
   ASSERT_TRUE(fromspace.Contains(*ptr));

   // the object should be in the first page, and the page should be marked
   static const int64_t kFirstPageIndex = 0;
   ASSERT_TRUE(zone->IsPageMarked(kFirstPageIndex));
   ASSERT_TRUE(zone->GetPageAt(kFirstPageIndex)->Contains(*ptr));
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

   auto p1 = zone->GetPageAt(1);
   ASSERT_NO_FATAL_FAILURE(zone->MarkPage(p1));
   auto p2 = zone->GetPageAt(3);
   ASSERT_NO_FATAL_FAILURE(zone->MarkPage(p2));
   auto p3 = zone->GetPageAt(5);
   ASSERT_NO_FATAL_FAILURE(zone->MarkPage(p3));

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

 TEST_F(NewZoneTest, TestVisitPointers) {
   MemoryRegion region(GetNewZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto zone = NewZone::New(region);

   static const constexpr word kAValue = 124;
   auto a = TryAllocateWord(zone, kAValue);
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsNewWord(a, kAValue));

   static const constexpr word kBValue = 512;
   auto b = TryAllocateWord(zone, kBValue);
   ASSERT_TRUE(IsAllocated(b));
   ASSERT_TRUE(IsNewWord(b, kBValue));

   MockRawObjectVisitor visitor;
   EXPECT_CALL(visitor, Visit(a))
    .Times(1)
    .WillOnce(Return(true));
   EXPECT_CALL(visitor, Visit(b))
    .Times(1)
    .WillOnce(Return(true));
   ASSERT_NO_FATAL_FAILURE(zone->VisitPointers(&visitor));
 }

 TEST_F(NewZoneTest, TestVisitMarkedPointers) {
   MemoryRegion region(GetNewZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto zone = NewZone::New(region);
   static const constexpr int64_t kNumberOfPointers = 3;
   static const constexpr int64_t kNumberOfMarkedPointers = 3;

   for(auto idx = 0; idx < kNumberOfPointers; idx++){
     auto ptr = TryAllocateWord(zone, idx);
     ASSERT_TRUE(IsAllocated(ptr));
     ASSERT_TRUE(IsNew(ptr));
     ASSERT_TRUE(IsWord(ptr, idx));
   }

   for(auto idx = 0; idx < kNumberOfMarkedPointers; idx++){
     auto ptr = TryAllocateMarkedWord(zone, idx);
     ASSERT_TRUE(IsAllocated(ptr));
     ASSERT_TRUE(IsNew(ptr));
     ASSERT_TRUE(IsMarked(ptr));
     ASSERT_TRUE(IsWord(ptr, idx));
   }

   MockRawObjectVisitor visitor;
   EXPECT_CALL(visitor, Visit)
     .Times(kNumberOfMarkedPointers);
   ASSERT_NO_FATAL_FAILURE(zone->VisitMarkedPointers(&visitor));
 }
}