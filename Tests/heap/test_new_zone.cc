#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "helpers.h"
#include "poseidon/heap/new_zone.h"

namespace poseidon{
#define UNALLOCATED 0 //TODO: cleanup

 using namespace ::testing;

 class NewZoneTest : public Test {
  protected:
   NewZoneTest() = default;

   static inline void
   SwapSpaces(NewZone& zone) {
     zone.SwapSpaces();
   }

   static inline Semispace
   GetFromspace(const NewZone& zone) {
     return zone.GetFromspace();
   }

   static inline Semispace
   GetTospace(const NewZone& zone) {
     return zone.GetTospace();
   }

   static inline uword
   TryAllocateBytes(NewZone& zone, const ObjectSize size) {
     return zone.TryAllocate(size);
   }

   static inline RawObject*
   TryAllocateWord(NewZone& zone, word value) {
     auto address = TryAllocateBytes(zone, kWordSize);
     if (address == UNALLOCATED)
       return nullptr;
     auto ptr = (RawObject*)address;
     (*((word*)ptr->GetObjectPointerAddress())) = value;
     return ptr;
   }

   static inline RawObject*
   TryAllocateMarkedWord(NewZone& zone, word value) {
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

 TEST_F(NewZoneTest, TestConstructor) {
   MemoryRegion region(GetNewZoneSize());
   NewZone zone(region);
   ASSERT_EQ((const Region&)zone, (const Region&)region);

   const int64_t semi_size = region.GetSize() / 2;
   ASSERT_EQ(zone.semisize(), semi_size);

   const uword fromspace = region.GetStartingAddress();
   ASSERT_EQ(zone.fromspace(), fromspace);

   const uword tospace = region.GetStartingAddress() + semi_size;
   ASSERT_EQ(zone.tospace(), tospace);

   const auto page_size = GetNewPageSize();
   const auto num_pages = GetNumberOfNewPages();
   for(auto idx = 0; idx < num_pages; idx++) {
     auto page_start = region.GetStartingAddress() + (idx * page_size);
     auto page = zone.pages(idx);
     LOG(INFO) << "validating " << (*page) << " in " << zone;
     ASSERT_EQ(page->index(), idx);
     ASSERT_TRUE(page->IsEmpty());
     ASSERT_FALSE(page->marked());
     ASSERT_EQ(page->GetStartingAddress(), page_start);
     ASSERT_EQ(page->GetSize(), page_size);
   }
 }

 TEST_F(NewZoneTest, TestEquals) {
   MemoryRegion region(GetNewZoneSize());

   NewZone a(region);
   NewZone b(region);
   ASSERT_EQ(a, b);
 }

 TEST_F(NewZoneTest, TestTryAllocateWillFail_SizeLessThanZero) {
   MemoryRegion region(GetNewZoneSize());
   NewZone zone(region);
   auto ptr = TryAllocateBytes(zone, -1);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(NewZoneTest, TestTryAllocateWillFail_SizeEqualToZero) {
   MemoryRegion region(GetNewZoneSize());
   NewZone zone(region);
   auto ptr = TryAllocateBytes(zone, 0);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(NewZoneTest, TestTryAllocateWillFail_SizeEqualToZoneSize) {
   MemoryRegion region(GetNewZoneSize());
   NewZone zone(region);
   auto ptr = TryAllocateBytes(zone, GetNewZoneSize());
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(NewZoneTest, TestTryAllocateWillFail_SizeGreaterThanZoneSize) {
   MemoryRegion region(GetNewZoneSize());
   NewZone zone(region);
   auto ptr = TryAllocateBytes(zone, GetNewZoneSize() + 1);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(NewZoneTest, TestTryAllocate_WillPass){
   MemoryRegion region(GetNewZoneSize());
   NewZone zone(region);
   Semispace fromspace = GetFromspace(zone);

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
   auto first_page = (*zone.pages());
   ASSERT_TRUE(first_page.Contains(*ptr));
   ASSERT_TRUE(first_page.marked());
   ASSERT_TRUE(zone.IsMarked(first_page.index()));
 }

 TEST_F(NewZoneTest, TestVisitPages) {
   MemoryRegion region(GetNewZoneSize());
   NewZone zone(region);

   MockPageVisitor visitor;
   EXPECT_CALL(visitor, VisitPage)
    .Times(static_cast<int>(zone.GetNumberOfPages()));
   ASSERT_NO_FATAL_FAILURE(zone.VisitPages(&visitor));
 }

 TEST_F(NewZoneTest, TestVisitMarkedPages) {
   MemoryRegion region(GetNewZoneSize());
   NewZone zone(region);

   static const constexpr int kNumberOfMarkedPages = 3;
   ASSERT_TRUE(zone.Mark(0));
   ASSERT_TRUE(zone.Mark(1));
   ASSERT_TRUE(zone.Mark(2));

   MockPageVisitor visitor;
   EXPECT_CALL(visitor, VisitPage)
       .Times(kNumberOfMarkedPages);
   ASSERT_NO_FATAL_FAILURE(zone.VisitMarkedPages(&visitor));
 }

 TEST_F(NewZoneTest, TestVisitPointers) {
   MemoryRegion region(GetNewZoneSize());
   NewZone zone(region);
   static const constexpr int64_t kNumberOfPointers = 3;

   for(auto idx = 0; idx < kNumberOfPointers; idx++){
     auto ptr = TryAllocateWord(zone, idx);
     ASSERT_TRUE(IsAllocated(ptr));
     ASSERT_TRUE(IsNew(ptr));
     ASSERT_TRUE(IsWord(ptr, idx));
   }

   MockRawObjectVisitor visitor;
   EXPECT_CALL(visitor, Visit)
     .Times(kNumberOfPointers);
   ASSERT_NO_FATAL_FAILURE(zone.VisitPointers(&visitor));
 }

 TEST_F(NewZoneTest, TestVisitMarkedPointers) {
   MemoryRegion region(GetNewZoneSize());
   NewZone zone(region);
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
   ASSERT_NO_FATAL_FAILURE(zone.VisitMarkedPointers(&visitor));
 }
}