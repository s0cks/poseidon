#include <gtest/gtest.h>

#include "poseidon/object.h"
#include "poseidon/heap/semispace.h"

#include "helpers.h"

#include "poseidon/marker/marker.h"

#include "assertions/ptr_assertions.h"
#include "assertions/type_assertions.h"

#include "mock_raw_object_visitor.h"

namespace poseidon{
 using namespace ::testing;

 class SemispaceTest : public Test{
  protected:
   SemispaceTest() = default;

   static inline void
   SetCurrentAddress(Semispace& semispace, const uword address) {
     ASSERT_TRUE(semispace.Contains(address));
     semispace.current_ = address;
   }
  public:
   ~SemispaceTest() override = default;
 };

 TEST_F(SemispaceTest, TestConstructor_WillPass){
   MemoryRegion region(flags::GetNewZoneSemispaceSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   Semispace semispace(Space::kUnknownSpace, region);
   ASSERT_EQ(semispace.GetStartingAddress(), region.GetStartingAddress());
   ASSERT_TRUE(semispace.IsEmpty());
   ASSERT_EQ(semispace.GetCurrentAddress(), semispace.GetCurrentAddress());
   ASSERT_EQ(semispace.GetSize(), region.GetSize());
 }

 TEST_F(SemispaceTest, TestSwap_WillPass){
   // a == r1
   MemoryRegion r1(flags::GetNewZoneSemispaceSize());
   ASSERT_TRUE(r1.Protect(MemoryRegion::kReadWrite));
   Semispace a(Space::kUnknownSpace, r1);
   ASSERT_EQ(a.GetStartingAddress(), r1.GetStartingAddress());
   ASSERT_EQ(a.GetCurrentAddress(), r1.GetStartingAddress());
   ASSERT_EQ(a.GetSize(), r1.GetSize());

   static constexpr const word kAValue = 1034;
   auto ptr = Int32::TryAllocateIn<>(&a, kAValue);
   ASSERT_NE(ptr, nullptr);
   ASSERT_TRUE(IsInt32(ptr));
   ASSERT_TRUE(Int32Eq(kAValue, ptr));
   ASSERT_TRUE(a.Intersects((Region) *ptr->raw_ptr()));

   // b == r2
   MemoryRegion r2(flags::GetNewZoneSemispaceSize());
   ASSERT_TRUE(r2.Protect(MemoryRegion::kReadWrite));
   Semispace b(Space::kUnknownSpace, r2);
   ASSERT_EQ(b.GetStartingAddress(), r2.GetStartingAddress());
   ASSERT_EQ(b.GetCurrentAddress(), r2.GetStartingAddress());
   ASSERT_EQ(b.GetSize(), r2.GetSize());
   ASSERT_FALSE(b.Intersects((Region) *ptr->raw_ptr()));

   ASSERT_NE(a, b);
   ASSERT_NO_FATAL_FAILURE(std::swap(a, b));
   ASSERT_TRUE(IsInt32(ptr->raw_ptr()));
   ASSERT_TRUE(Int32Eq(kAValue, ptr));

   // a == r2
   ASSERT_EQ(a.GetStartingAddress(), r2.GetStartingAddress());
   ASSERT_EQ(a.GetCurrentAddress(), r2.GetStartingAddress());
   ASSERT_EQ(a.GetSize(), r2.GetSize());
   ASSERT_FALSE(a.Intersects((Region) *ptr->raw_ptr()));

   // b == r1
   ASSERT_EQ(b.GetStartingAddress(), r1.GetStartingAddress());
   ASSERT_EQ(b.GetCurrentAddress(), r1.GetStartingAddress() + ptr->raw_ptr()->GetTotalSize());
   ASSERT_EQ(b.GetSize(), r1.GetSize());
   ASSERT_TRUE(b.Intersects((Region) *ptr->raw_ptr()));
 }

#define DEFINE_TRY_AlLOCATE_BYTES_FAILS_SEMISPACE_TEST(TestName, NumberOfBytes) \
  DEFINE_TRY_ALLOCATE_BYTES_FAILS_TEST(SemispaceTest, TestName, Semispace, flags::GetNewZoneSemispaceSize(), NumberOfBytes)

 DEFINE_TRY_AlLOCATE_BYTES_FAILS_SEMISPACE_TEST(SizeLessThanZero, -1);
 DEFINE_TRY_AlLOCATE_BYTES_FAILS_SEMISPACE_TEST(SizeEqualsZero, 0);
 DEFINE_TRY_AlLOCATE_BYTES_FAILS_SEMISPACE_TEST(SizeLessThanMin, Semispace::GetMinimumObjectSize() - 1);
 DEFINE_TRY_AlLOCATE_BYTES_FAILS_SEMISPACE_TEST(SizeGreaterThanMax, Semispace::GetMaximumObjectSize() + 1);
 DEFINE_TRY_AlLOCATE_BYTES_FAILS_SEMISPACE_TEST(SizeEqualToSemispaceSize, flags::GetNewZoneSemispaceSize());
 DEFINE_TRY_AlLOCATE_BYTES_FAILS_SEMISPACE_TEST(SizeGreaterThanSemispaceSize, flags::GetNewZoneSemispaceSize() + 1);

 TEST_F(SemispaceTest, TestTryAllocateBytes_WillFail_NotEnoughSpace){
   auto semispace = Semispace(flags::GetNewZoneSemispaceSize());
   ASSERT_NO_FATAL_FAILURE(semispace.SetWritable());
   SetCurrentAddress(semispace, semispace.GetEndingAddress());
   auto new_ptr = semispace.TryAllocateBytes(kWordSize);
   ASSERT_EQ(new_ptr, UNALLOCATED);
 }

 TEST_F(SemispaceTest, TestTryAllocate){
   auto semispace = Semispace(flags::GetNewZoneSemispaceSize());
   ASSERT_NO_FATAL_FAILURE(semispace.SetWritable());

   static const constexpr RawInt32 kDefaultValue = 42;
   auto ptr = Int32::TryAllocateIn<>(&semispace, kDefaultValue);
   ASSERT_NE(ptr, nullptr);
   ASSERT_TRUE(IsAllocated(ptr->raw_ptr()));
   ASSERT_TRUE(IsNew(ptr->raw_ptr()));
   ASSERT_FALSE(IsOld(ptr->raw_ptr()));
   ASSERT_FALSE(IsMarked(ptr->raw_ptr()));
   ASSERT_FALSE(IsRemembered(ptr->raw_ptr()));
   ASSERT_FALSE(IsForwarding(ptr->raw_ptr()));
   ASSERT_TRUE(IsInt32(ptr->raw_ptr()));
   ASSERT_TRUE(Int32Eq(kDefaultValue, ptr));

   ASSERT_TRUE(SemispacePrinter::Print(&semispace));
 }

 TEST_F(SemispaceTest, TestVisitPointers){
   auto semispace = Semispace(flags::GetNewZoneSemispaceSize());
   ASSERT_NO_FATAL_FAILURE(semispace.SetWritable());

   static const constexpr int64_t kNumberOfPointers = 3;
   for(RawInt32 idx = 0; idx < kNumberOfPointers; idx++){
     auto ptr = Int32::TryAllocateIn(&semispace, idx);
     ASSERT_NE(ptr, nullptr);
     ASSERT_TRUE(IsAllocated(ptr->raw_ptr()));
     ASSERT_TRUE(IsNew(ptr->raw_ptr()));
     ASSERT_FALSE(IsOld(ptr->raw_ptr()));
     ASSERT_FALSE(IsMarked(ptr->raw_ptr()));
     ASSERT_FALSE(IsRemembered(ptr->raw_ptr()));
     ASSERT_FALSE(IsForwarding(ptr->raw_ptr()));
     ASSERT_TRUE(IsInt32(ptr->raw_ptr()));
     ASSERT_TRUE(Int32Eq(idx, ptr));
   }

   MockRawObjectVisitor visitor;
   EXPECT_CALL(visitor, Visit)
     .Times(kNumberOfPointers);
   ASSERT_NO_FATAL_FAILURE(semispace.VisitPointers(&visitor));

   ASSERT_NO_FATAL_FAILURE(SemispacePrinter::Print(&semispace));
 }

 TEST_F(SemispaceTest, TestVisitMarkedPointers){
   auto semispace = Semispace(flags::GetNewZoneSemispaceSize());
   ASSERT_NO_FATAL_FAILURE(semispace.SetWritable());

   static const constexpr int64_t kNumberOfUnmarkedPointers = 1;
   static const constexpr int64_t kNumberOfMarkedPointers = 3;

   for(RawInt32 idx = 0; idx < kNumberOfUnmarkedPointers; idx++){
     auto ptr = Int32::TryAllocateIn(&semispace, idx);
     ASSERT_NE(ptr, nullptr);
     ASSERT_TRUE(IsAllocated(ptr->raw_ptr()));
     ASSERT_TRUE(IsNew(ptr->raw_ptr()));
     ASSERT_FALSE(IsOld(ptr->raw_ptr()));
     ASSERT_FALSE(IsMarked(ptr->raw_ptr()));
     ASSERT_FALSE(IsRemembered(ptr->raw_ptr()));
     ASSERT_FALSE(IsForwarding(ptr->raw_ptr()));
     ASSERT_TRUE(IsInt32(ptr->raw_ptr()));
     ASSERT_TRUE(Int32Eq(idx, ptr));
   }

   Marker marker;
   for(RawInt32 idx = 0; idx < kNumberOfMarkedPointers; idx++){
     auto ptr = Int32::TryAllocateIn(&semispace, idx);

     ASSERT_NE(ptr, nullptr);
     ASSERT_TRUE(IsAllocated(ptr->raw_ptr()));
     ASSERT_TRUE(IsNew(ptr->raw_ptr()));
     ASSERT_FALSE(IsOld(ptr->raw_ptr()));
     ASSERT_TRUE(IsMarked(ptr->raw_ptr()));
     ASSERT_FALSE(IsRemembered(ptr->raw_ptr()));
     ASSERT_FALSE(IsForwarding(ptr->raw_ptr()));
     ASSERT_TRUE(IsInt32(ptr->raw_ptr()));
     ASSERT_TRUE(Int32Eq(idx, ptr));
   }

   MockRawObjectVisitor visitor;
   EXPECT_CALL(visitor, Visit)
     .Times(kNumberOfMarkedPointers);
   ASSERT_NO_FATAL_FAILURE(semispace.VisitMarkedPointers(&visitor));

   ASSERT_NO_FATAL_FAILURE(SemispacePrinter::Print(&semispace));
 }
}