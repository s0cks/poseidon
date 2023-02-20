#include "zone/test_new_zone.h"
#include "poseidon/flags.h"
#include "poseidon/object.h"

#include "helpers.h"
#include "matchers/is_pointer_to.h"
#include "assertions/ptr_assertions.h"
#include "assertions/type_assertions.h"

namespace poseidon {
 using namespace ::testing;

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
}