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
   ASSERT_EQ(zone.GetCurrentAddress(), zone.GetStartingAddress());

   const int64_t semi_size = region.GetSize() / 2;
   ASSERT_EQ(zone.semisize(), semi_size);

   const uword fromspace = region.GetStartingAddress();
   ASSERT_EQ(zone.fromspace(), fromspace);

   const uword tospace = region.GetStartingAddress() + semi_size;
   ASSERT_EQ(zone.tospace(), tospace);
 }

 TEST_F(NewZoneTest, TestEquals) {
   MemoryRegion region(GetNewZoneSize());

   NewZone a(region);
   NewZone b(region);
   ASSERT_EQ(a, b);
 }

 TEST_F(NewZoneTest, TestNotEquals) {
   NOT_IMPLEMENTED(ERROR);
   ASSERT_TRUE(false);//TODO: implement
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

 TEST_F(NewZoneTest, TestTryAllocate){
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

   //TODO: assert fromspace contains ptr
   //TODO: assert pages(1) is marked
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