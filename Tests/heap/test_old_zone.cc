#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "helpers.h"
#include "mock_raw_object_visitor.h"
#include "poseidon/heap/old_zone.h"
#include "heap/mock_old_page_visitor.h"

namespace poseidon {
#define UNALLOCATED 0 //TODO: cleanup

 using namespace ::testing;

 class OldZoneTest : public Test {
  protected:
   OldZoneTest() = default;

   static inline uword
   TryAllocateBytes(OldZone* zone, const ObjectSize size) {
     return zone->TryAllocate(size);
   }

   static inline RawObject*
   TryAllocateWord(OldZone* zone, word value) {
     auto address = TryAllocateBytes(zone, kWordSize);
     if (address == UNALLOCATED)
       return nullptr;
     auto ptr = (RawObject*)address;
     (*((word*)ptr->GetObjectPointerAddress())) = value;
     return ptr;
   }

   static inline RawObject*
   TryAllocateMarkedWord(OldZone* zone, word value) {
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
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto zone = OldZone::From(region);
   ASSERT_EQ(zone->GetStartingAddress(), region.GetStartingAddress());
   ASSERT_EQ(zone->GetSize(), region.GetSize());
 }

 TEST_F(OldZoneTest, TestGetPageAt_WillFail_IndexLessThanZero) {

 }

 //TODO: add equals & not equals tests

 TEST_F(OldZoneTest, TestTryAllocateWillFail_SizeLessThanZero) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto zone = OldZone::From(region);
   auto ptr = TryAllocateBytes(zone, -1);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(OldZoneTest, TestTryAllocateWillFail_SizeEqualToZero) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto zone = OldZone::From(region);
   auto ptr = TryAllocateBytes(zone, 0);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(OldZoneTest, TestTryAllocateWillFail_SizeEqualToZoneSize) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto zone = OldZone::From(region);
   auto ptr = TryAllocateBytes(zone, GetOldZoneSize());
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(OldZoneTest, TestTryAllocateWillFail_SizeGreaterThanZoneSize) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto zone = OldZone::From(region);
   auto ptr = TryAllocateBytes(zone, GetOldZoneSize() + 1);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(OldZoneTest, TestTryAllocate){
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto zone = OldZone::From(region);

   static const constexpr word kDefaultWordValue = 42;
   auto ptr = TryAllocateWord(zone, kDefaultWordValue);
   ASSERT_TRUE(IsAllocated(ptr));
   ASSERT_FALSE(IsNew(ptr));
   ASSERT_TRUE(IsOld(ptr));
   ASSERT_FALSE(IsMarked(ptr));
   ASSERT_FALSE(IsRemembered(ptr));
   ASSERT_FALSE(IsForwarding(ptr));
   ASSERT_TRUE(IsWord(ptr, kDefaultWordValue));
 }

 TEST_F(OldZoneTest, TestVisitPages) {
//TODO:
//   MemoryRegion region(GetOldZoneSize());
//   OldZone zone(region);
//
//   MockPageVisitor visitor;
//   EXPECT_CALL(visitor, VisitPage)
//       .Times(static_cast<int>(zone.GetNumberOfPages()));
//   ASSERT_NO_FATAL_FAILURE(zone.VisitPages(&visitor));
 }

 TEST_F(OldZoneTest, TestVisitMarkedPages) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto zone = OldZone::From(region);

   static const constexpr int kNumberOfMarkedPages = 3;
   ASSERT_NO_FATAL_FAILURE(zone->Mark((const int64_t) 0));
   ASSERT_NO_FATAL_FAILURE(zone->Mark(1));
   ASSERT_NO_FATAL_FAILURE(zone->Mark(2));

   MockOldPageVisitor visitor;
   EXPECT_CALL(visitor, Visit)
     .Times(kNumberOfMarkedPages);
   ASSERT_NO_FATAL_FAILURE(zone->VisitMarkedPages(&visitor));
 }

 TEST_F(OldZoneTest, TestVisitPointers) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto zone = OldZone::From(region);

   static const constexpr int64_t kNumberOfPointers = 3;
   for(auto idx = 0; idx < kNumberOfPointers; idx++){
     auto ptr = TryAllocateWord(zone, idx);
     ASSERT_TRUE(IsAllocated(ptr));
     ASSERT_TRUE(IsOld(ptr));
     ASSERT_TRUE(IsWord(ptr, idx));
   }

   MockRawObjectVisitor visitor;
   EXPECT_CALL(visitor, Visit)
       .Times(kNumberOfPointers + 1);
   ASSERT_NO_FATAL_FAILURE(zone->VisitPointers(&visitor));
 }

 TEST_F(OldZoneTest, TestVisitMarkedPointers) {
   MemoryRegion region(GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto zone = OldZone::From(region);
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
   ASSERT_NO_FATAL_FAILURE(zone->VisitMarkedPointers(&visitor));
 }
}