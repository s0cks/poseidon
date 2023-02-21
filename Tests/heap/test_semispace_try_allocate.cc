#include "heap/test_semispace.h"

#include "poseidon/flags.h"
#include "poseidon/object.h"

#include "matchers/is_pointer_to.h"
#include "assertions/ptr_assertions.h"
#include "assertions/type_assertions.h"

namespace poseidon {
 using namespace ::testing;

 TEST_F(SemispaceTest, TestTryAllocate_WillFail_NotEnoughSpace) {
   ASSERT_TRUE(semispace().IsEmpty());

   ASSERT_NO_FATAL_FAILURE(SetCurrentAddress(semispace().GetEndingAddress()));
   static const RawInt32 kBValue = 0xB;
   auto a = Int32::TryAllocateIn(&semispace(), kBValue);
   ASSERT_TRUE(IsUnallocated(a));
   ASSERT_FALSE(IsAllocated(a));
 }

#define DEFINE_TRY_ALLOCATE_FAILS_TEST(Name, NumberOfBytes) \
 TEST_F(SemispaceTest, TestTryAllocate_WillFail_##Name) {   \
   auto new_ptr = TryAllocatePointer(NumberOfBytes);        \
   ASSERT_TRUE(IsUnallocated(new_ptr));                     \
 }

#define DEFINE_TRY_ALLOCATE_PASSES_TEST(Name, NumberOfBytes) \
 TEST_F(SemispaceTest, TestTryAllocate_WillPass_##Name) {    \
   auto new_ptr = TryAllocatePointer(NumberOfBytes);         \
   ASSERT_TRUE(IsAllocated(new_ptr));                        \
   ASSERT_TRUE(IsNew(new_ptr));                              \
   ASSERT_FALSE(IsOld(new_ptr));                             \
   ASSERT_FALSE(IsRemembered(new_ptr));                      \
   ASSERT_FALSE(IsMarked(new_ptr));                          \
   ASSERT_FALSE(IsForwarding(new_ptr));                      \
 }

 DEFINE_TRY_ALLOCATE_FAILS_TEST(SizeLessThanZero, -1);
 DEFINE_TRY_ALLOCATE_FAILS_TEST(SizeEqualsZero, 0);
 DEFINE_TRY_ALLOCATE_FAILS_TEST(SizeLessThanMin, Semispace::GetMinimumObjectSize() - 1);
 DEFINE_TRY_ALLOCATE_FAILS_TEST(SizeEqualsZoneSize, flags::GetNewZoneSize());
 DEFINE_TRY_ALLOCATE_FAILS_TEST(SizeGreaterThanSemispaceSize, flags::GetNewZoneSemispaceSize() + 1);
 DEFINE_TRY_ALLOCATE_FAILS_TEST(SizeGreaterThanMax, Semispace::GetMaximumObjectSize() + 1);

 DEFINE_TRY_ALLOCATE_PASSES_TEST(SizeEqualsMin, NewZone::GetMinimumObjectSize());
 DEFINE_TRY_ALLOCATE_PASSES_TEST(SizeEqualsMax, NewZone::GetMaximumObjectSize());

 TEST_F(SemispaceTest, TestTryAllocate_Null_WillPass) {
   auto ptr = Null::TryAllocateIn(&semispace());
   ASSERT_TRUE(IsAllocated(ptr));
   ASSERT_TRUE(IsNull(ptr));
   ASSERT_TRUE(IsNew(ptr));
   ASSERT_FALSE(IsOld(ptr));
   ASSERT_FALSE(IsFree(ptr));
   ASSERT_FALSE(IsMarked(ptr));
   ASSERT_FALSE(IsRemembered(ptr));
   ASSERT_FALSE(IsForwarding(ptr));
 }

 TEST_F(SemispaceTest, TestTryAllocate_True_WillPass) {
   auto ptr = Bool::TryAllocateIn(&semispace(), true);
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

 TEST_F(SemispaceTest, TestTryAllocate_False_WillPass) {
   auto ptr = Bool::TryAllocateIn(&semispace(), false);
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

 TEST_F(SemispaceTest, TestTryAllocate_Tuple_WillPass) {
   auto ptr = Tuple::TryAllocateIn(&semispace());
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
 TEST_F(SemispaceTest, TestTryAllocate_##Type##_WillPass) {        \
   static constexpr const Raw##Type kValue = 0xA;                  \
   auto ptr = Type::TryAllocateIn(&semispace(), kValue);           \
   ASSERT_TRUE(IsAllocated(ptr));                                  \
   ASSERT_TRUE(Is##Type(ptr));                                     \
   ASSERT_TRUE(Type##Eq(kValue, ptr));                             \
   ASSERT_TRUE(IsNew(ptr));                                        \
   ASSERT_FALSE(IsOld(ptr));                                       \
   ASSERT_FALSE(IsFree(ptr));                                      \
   ASSERT_FALSE(IsMarked(ptr));                                    \
   ASSERT_FALSE(IsRemembered(ptr));                                \
   ASSERT_FALSE(IsForwarding(ptr));                                \
 }

 FOR_EACH_INT_TYPE(DEFINE_TRY_ALLOCATE_NUMBER_TYPE_PASSES_NEW_ZONE_TEST);
}