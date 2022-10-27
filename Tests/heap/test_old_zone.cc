#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "helpers.h"
#include "poseidon/heap/old_zone.h"

namespace poseidon {
#define UNALLOCATED 0 //TODO: cleanup

 using namespace ::testing;

 class OldZoneTest : public Test {
  protected:
   OldZoneTest() = default;

   static inline uword
   TryAllocateBytes(OldZone& zone, const ObjectSize size) {
     return zone.TryAllocate(size);
   }

   static inline RawObject*
   TryAllocateWord(OldZone& zone, word value) {
     auto address = TryAllocateBytes(zone, kWordSize);
     if (address == UNALLOCATED)
       return nullptr;
     auto ptr = (RawObject*)address;
     (*((word*)ptr->GetObjectPointerAddress())) = value;
     return ptr;
   }

   static inline RawObject*
   TryAllocateMarkedWord(OldZone& zone, word value) {
     auto address = TryAllocateBytes(zone, kWordSize);
     if (address == UNALLOCATED)
       return nullptr;
     auto ptr = (RawObject*)address;
     ptr->SetMarkedBit();
     (*((word*)ptr->GetObjectPointerAddress())) = value;
     return ptr;
   }
  public:
   ~OldZoneTest() override = default;
 };

 TEST_F(OldZoneTest, TestConstructor) {
   MemoryRegion region(GetOldZoneSize());
   OldZone zone(region);
   ASSERT_EQ((const Region&)zone, (const Region&)region);
   ASSERT_EQ(zone.GetSize(), GetOldZoneSize());

   const auto page_size = GetOldPageSize();
   const auto num_pages = GetNumberOfNewPages();
   for(auto idx = 0; idx < num_pages; idx++) {
     auto page_start = region.GetStartingAddress() + (idx * page_size);
     auto page = zone.pages(idx);
     DLOG(INFO) << "checking: " << (*page);
     ASSERT_EQ(page->index(), idx);
     ASSERT_TRUE(page->IsEmpty());
     ASSERT_FALSE(page->marked());
     ASSERT_EQ(page->GetStartingAddress(), page_start);
     ASSERT_EQ(page->GetSize(), page_size);
   }
 }

 TEST_F(OldZoneTest, TestEquals) {
   MemoryRegion region(GetOldZoneSize());

   OldZone a(region);
   OldZone b(region);
   ASSERT_EQ(a, b);
 }

 TEST_F(OldZoneTest, TestTryAllocateWillFail_SizeLessThanZero) {
   MemoryRegion region(GetOldZoneSize());
   OldZone zone(region);
   auto ptr = TryAllocateBytes(zone, -1);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(OldZoneTest, TestTryAllocateWillFail_SizeEqualToZero) {
   MemoryRegion region(GetOldZoneSize());
   OldZone zone(region);
   auto ptr = TryAllocateBytes(zone, 0);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(OldZoneTest, TestTryAllocateWillFail_SizeEqualToZoneSize) {
   MemoryRegion region(GetOldZoneSize());
   OldZone zone(region);
   auto ptr = TryAllocateBytes(zone, GetOldZoneSize());
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(OldZoneTest, TestTryAllocateWillFail_SizeGreaterThanZoneSize) {
   MemoryRegion region(GetOldZoneSize());
   OldZone zone(region);
   auto ptr = TryAllocateBytes(zone, GetOldZoneSize() + 1);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(OldZoneTest, TestTryAllocate){
   MemoryRegion region(GetNewZoneSize());
   OldZone zone(region);

   static const constexpr word kDefaultWordValue = 42;
   auto ptr = TryAllocateWord(zone, kDefaultWordValue);
   ASSERT_TRUE(IsAllocated(ptr));
   ASSERT_FALSE(IsNew(ptr));
   ASSERT_TRUE(IsOld(ptr));
   ASSERT_FALSE(IsMarked(ptr));
   ASSERT_FALSE(IsRemembered(ptr));
   ASSERT_FALSE(IsForwarding(ptr));
   ASSERT_TRUE(IsWord(ptr, kDefaultWordValue));
   ASSERT_TRUE(zone.IsMarked(0));
 }

 TEST_F(OldZoneTest, TestVisitPages) {
   MemoryRegion region(GetOldZoneSize());
   OldZone zone(region);

   MockPageVisitor visitor;
   EXPECT_CALL(visitor, VisitPage)
       .Times(static_cast<int>(zone.GetNumberOfPages()));
   ASSERT_NO_FATAL_FAILURE(zone.VisitPages(&visitor));
 }

 TEST_F(OldZoneTest, TestVisitMarkedPages) {
   MemoryRegion region(GetOldZoneSize());
   OldZone zone(region);

   static const constexpr int kNumberOfMarkedPages = 3;
   ASSERT_TRUE(zone.Mark(0));
   ASSERT_TRUE(zone.Mark(1));
   ASSERT_TRUE(zone.Mark(2));

   MockPageVisitor visitor;
   EXPECT_CALL(visitor, VisitPage)
       .Times(kNumberOfMarkedPages);
   ASSERT_NO_FATAL_FAILURE(zone.VisitMarkedPages(&visitor));
 }

 TEST_F(OldZoneTest, TestVisitPointers) {
   MemoryRegion region(GetNewZoneSize());
   OldZone zone(region);
   static const constexpr int64_t kNumberOfPointers = 3;

   for(auto idx = 0; idx < kNumberOfPointers; idx++){
     auto ptr = TryAllocateWord(zone, idx);
     ASSERT_TRUE(IsAllocated(ptr));
     ASSERT_TRUE(IsOld(ptr));
     ASSERT_TRUE(IsWord(ptr, idx));
   }

   MockRawObjectVisitor visitor;
   EXPECT_CALL(visitor, Visit)
       .Times(kNumberOfPointers);
   ASSERT_NO_FATAL_FAILURE(zone.VisitPointers(&visitor));
 }

 TEST_F(OldZoneTest, TestVisitMarkedPointers) {
   MemoryRegion region(GetNewZoneSize());
   OldZone zone(region);
   static const constexpr int64_t kNumberOfPointers = 3;
   static const constexpr int64_t kNumberOfMarkedPointers = 3;

   for(auto idx = 0; idx < kNumberOfPointers; idx++){
     auto ptr = TryAllocateWord(zone, idx);
     ASSERT_TRUE(IsAllocated(ptr));
     ASSERT_TRUE(IsOld(ptr));
     ASSERT_TRUE(IsWord(ptr, idx));
   }

   for(auto idx = 0; idx < kNumberOfMarkedPointers; idx++){
     auto ptr = TryAllocateMarkedWord(zone, idx);
     ASSERT_TRUE(IsAllocated(ptr));
     ASSERT_TRUE(IsOld(ptr));
     ASSERT_TRUE(IsMarked(ptr));
     ASSERT_TRUE(IsWord(ptr, idx));
   }

   MockRawObjectVisitor visitor;
   EXPECT_CALL(visitor, Visit)
       .Times(kNumberOfMarkedPointers);
   ASSERT_NO_FATAL_FAILURE(zone.VisitMarkedPointers(&visitor));
 }
}