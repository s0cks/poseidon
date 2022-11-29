#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "helpers.h"
#include "mock_raw_object_visitor.h"
#include "poseidon/heap/zone/old_zone.h"
#include "heap/mock_old_page_visitor.h"

#include "poseidon/type/int.h"
#include "matchers/is_pointer_to.h"
#include "assertions/type_assertions.h"

namespace poseidon {
 using namespace ::testing;

 class OldZoneTest : public Test {
  protected:
   OldZoneTest() = default;

   static inline uword
   TryAllocateBytes(OldZone* zone, const ObjectSize size) {
     return zone->TryAllocateBytes(size);
   }

   static inline Pointer*
   TryAllocateWord(OldZone* zone, word value) {
     auto address = TryAllocateBytes(zone, kWordSize);
     if (address == UNALLOCATED)
       return nullptr;
     auto ptr = (Pointer*)address;
     (*((word*)ptr->GetObjectPointerAddress())) = value;
     return ptr;
   }

   static inline Pointer*
   TryAllocateMarkedWord(OldZone* zone, word value) {
     auto address = TryAllocateBytes(zone, kWordSize);
     if (address == UNALLOCATED)
       return nullptr;
     auto ptr = (Pointer*)address;
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
   OldZone zone(region);
   ASSERT_EQ(zone.GetStartingAddress(), region.GetStartingAddress());
   ASSERT_EQ(zone.GetSize(), region.GetSize());

   //TODO: check pages
 }

#define DEFINE_TRY_ALLOCATE_BYTES_FAILS_OLD_ZONE_TEST(TestName, NumberOfBytes) \
 DEFINE_TRY_ALLOCATE_BYTES_FAILS_TEST(OldZoneTest, TestName, OldZone, GetOldZoneSize(), NumberOfBytes)

 DEFINE_TRY_ALLOCATE_BYTES_FAILS_OLD_ZONE_TEST(SizeLessThanZero, -1);
 DEFINE_TRY_ALLOCATE_BYTES_FAILS_OLD_ZONE_TEST(SizeEqualsZero, 0);
 DEFINE_TRY_ALLOCATE_BYTES_FAILS_OLD_ZONE_TEST(SizeLessThanMin, OldZone::GetMinimumObjectSize() - 1);
 DEFINE_TRY_ALLOCATE_BYTES_FAILS_OLD_ZONE_TEST(SizeEqualsZoneSize, GetOldZoneSize());
 DEFINE_TRY_ALLOCATE_BYTES_FAILS_OLD_ZONE_TEST(SizeGreaterThanZoneSize, GetOldZoneSize() + 1);
 DEFINE_TRY_ALLOCATE_BYTES_FAILS_OLD_ZONE_TEST(SizeGreaterThanMax, OldZone::GetMaximumObjectSize() + 1);

 TEST_F(OldZoneTest, TestTryAllocateBytes_WillPass_SizeEqualsWordSize) {
   OldZone zone(GetOldZoneSize());
   auto address = zone.TryAllocateBytes(kWordSize);
   ASSERT_NE(address, UNALLOCATED);
   ASSERT_EQ(*((word*) address), 0);
 }

 TEST_F(OldZoneTest, TestTryAllocateBytes_WillPass_SizeEqualsMin) {
   OldZone zone(GetOldZoneSize());
   auto address = zone.TryAllocateBytes(OldZone::GetMinimumObjectSize());
   ASSERT_NE(address, UNALLOCATED);
   ASSERT_EQ(*((word*) address), 0);
 }

 TEST_F(OldZoneTest, TestTryAllocateBytes_WillPass_SizeEqualsMax) {
   OldZone zone(GetOldZoneSize());
   auto address = zone.TryAllocateBytes(OldZone::GetMaximumObjectSize());
   ASSERT_NE(address, UNALLOCATED);
   ASSERT_EQ(*((word*) address), 0);
 }

#define DEFINE_TRY_ALLOCATE_TYPE_PASSES_OLD_ZONE_TEST(Type, Value) \
 TEST_F(OldZoneTest, TestTryAllocate_##Type##_WillPass) {                    \
   OldZone zone(GetOldZoneSize());                                           \
   auto new_ptr = Type::TryAllocateIn<>(&zone, Value);                       \
   ASSERT_NE(new_ptr, nullptr);                                              \
   ASSERT_TRUE(IsInt(new_ptr->raw_ptr()));                                   \
   ASSERT_TRUE(Type##Eq(Value, new_ptr));                                    \
 }

 DEFINE_TRY_ALLOCATE_TYPE_PASSES_OLD_ZONE_TEST(Int, 42);

 TEST_F(OldZoneTest, TestVisitPointers_WillPass) {
   OldZone zone(GetOldZoneSize());

   MockRawObjectVisitor visitor;
   static const constexpr int64_t kNumberOfPointers = 3;
   for(auto idx = 0; idx < kNumberOfPointers; idx++){
     auto new_ptr = Int::TryAllocateIn(&zone, idx);
     ASSERT_NE(new_ptr, nullptr);
     ASSERT_TRUE(IsInt(new_ptr->raw_ptr()));
     ASSERT_TRUE(IntEq(idx, new_ptr));

     ON_CALL(visitor, Visit(IsPointerTo<>(new_ptr)))
      .WillByDefault(Return(true));
     EXPECT_CALL(visitor, Visit(IsPointerTo<>(new_ptr)))
      .WillOnce(Return(true));
   }
   ASSERT_TRUE(zone.VisitPointers(&visitor));
 }

 TEST_F(OldZoneTest, TestVisitMarkedPointers_WillPass) {
   OldZone zone(GetOldZoneSize());

   MockRawObjectVisitor visitor;
   static const constexpr int64_t kNumberOfPointers = 3;
   for(auto idx = 0; idx < kNumberOfPointers; idx++){
     auto new_ptr = Int::TryAllocateIn(&zone, idx);
     ASSERT_NE(new_ptr, nullptr);
     ASSERT_TRUE(IsInt(new_ptr->raw_ptr()));
     ASSERT_TRUE(IntEq(idx, new_ptr));

     ON_CALL(visitor, Visit(IsPointerTo<>(new_ptr)))
      .WillByDefault(Return(false));
   }

   static const constexpr int64_t kNumberOfMarkedPointers = 3;
   for(auto idx = 0; idx < kNumberOfMarkedPointers; idx++){
     auto new_ptr = Int::TryAllocateIn(&zone, idx);
     ASSERT_NE(new_ptr, nullptr);
     ASSERT_TRUE(IsInt(new_ptr->raw_ptr()));
     ASSERT_TRUE(IntEq(idx, new_ptr));

     ON_CALL(visitor, Visit(IsPointerTo<>(new_ptr)))
      .WillByDefault(Return(true));
     EXPECT_CALL(visitor, Visit(IsPointerTo<>(new_ptr)))
      .WillRepeatedly(Return(true));
   }
   ASSERT_TRUE(zone.VisitMarkedPointers(&visitor));
 }
}