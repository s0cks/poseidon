#include "heap/test_semispace.h"
#include "poseidon/object.h"

#include "assertions/ptr_assertions.h"
#include "assertions/type_assertions.h"
#include "matchers/is_pointer_to.h"
#include "mock_raw_object_visitor.h"

namespace poseidon{
 using namespace ::testing;

 TEST_F(SemispaceTest, TestConstructor_WillPass){
   MemoryRegion region(flags::GetNewZoneSemispaceSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   Semispace semispace(Space::kUnknownSpace, region);
   ASSERT_EQ(semispace.GetStartingAddress(), region.GetStartingAddress());
   ASSERT_TRUE(semispace.IsEmpty());
   ASSERT_EQ(semispace.GetCurrentAddress(), semispace.GetCurrentAddress());
   ASSERT_EQ(semispace.GetSize(), region.GetSize());
 }

 TEST_F(SemispaceTest, TestGetStartingAddress) {
   ASSERT_EQ(region().GetStartingAddress(), semispace().GetStartingAddress());
 }

 TEST_F(SemispaceTest, TestGetEndingAddress) {
   ASSERT_EQ(region().GetEndingAddress(), semispace().GetEndingAddress());
 }

 TEST_F(SemispaceTest, TestIsEmpty) {
   ASSERT_TRUE(semispace().IsEmpty());

   static constexpr const RawInt32 kAValue = 0xA;
   auto a = Int32::TryAllocateIn(&semispace(), kAValue);
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsInt32(a));
   ASSERT_TRUE(Int32Eq(kAValue, a));

   ASSERT_FALSE(semispace().IsEmpty());
 }

 TEST_F(SemispaceTest, TestClear) {
   ASSERT_TRUE(semispace().IsEmpty());

   static constexpr const RawInt32 kAValue = 0xA;
   auto a = Int32::TryAllocateIn(&semispace(), kAValue);
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsInt32(a));
   ASSERT_TRUE(Int32Eq(kAValue, a));

   ASSERT_FALSE(semispace().IsEmpty());
   ASSERT_NO_FATAL_FAILURE(ClearSemispace());
   ASSERT_TRUE(semispace().IsEmpty());
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

 TEST_F(SemispaceTest, TestVisitPointers_WillPass_VisitsNothing) {
   MockRawObjectVisitor visitor;
   ON_CALL(visitor, Visit(_))
    .WillByDefault(Return(false));
   ASSERT_TRUE(semispace().VisitPointers(&visitor));
 }

 TEST_F(SemispaceTest, TestVisitPointers_WillPass_VisitsOne) {
   static constexpr const RawInt32 kAValue = 333;
   auto a = Int32::TryAllocateIn(&semispace(), kAValue);
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsInt32(a));
   ASSERT_TRUE(Int32Eq(kAValue, a));

   MockRawObjectVisitor visitor;
   EXPECT_CALL(visitor, Visit(IsPointerTo(a)));
   ASSERT_TRUE(semispace().VisitPointers(&visitor));
 }

 TEST_F(SemispaceTest, TestVisitPointers_WillPass_VisitsMultiple) {
   MockRawObjectVisitor visitor;

   static const constexpr int64_t kNumberOfPointers = 3;
   for(RawInt32 idx = 0; idx < kNumberOfPointers; idx++){
     auto ptr = Int32::TryAllocateIn(&semispace(), idx);
     ASSERT_TRUE(IsAllocated(ptr));
     ASSERT_TRUE(IsInt32(ptr));
     ASSERT_TRUE(Int32Eq(idx, ptr));
     EXPECT_CALL(visitor, Visit(IsPointerTo(ptr)));
   }

   ASSERT_TRUE(semispace().VisitPointers(&visitor));
 }

 TEST_F(SemispaceTest, TestVisitMarkedPointers_WillPass_VisitsNothing) {
   MockRawObjectVisitor visitor;
   ON_CALL(visitor, Visit(_))
     .WillByDefault(Return(false));
   ASSERT_TRUE(semispace().VisitMarkedPointers(&visitor));
 }

 TEST_F(SemispaceTest, TestVisitMarkedPointers_WillPass_VisitsOne) {
   static constexpr const RawInt32 kAValue = 333;
   auto a = Int32::TryAllocateIn(&semispace(), kAValue);
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsInt32(a));
   ASSERT_TRUE(Int32Eq(kAValue, a));
   ASSERT_NO_FATAL_FAILURE(Mark(a));

   MockRawObjectVisitor visitor;
   EXPECT_CALL(visitor, Visit(IsPointerTo(a)));
   ASSERT_TRUE(semispace().VisitMarkedPointers(&visitor));
 }

 TEST_F(SemispaceTest, TestVisitMarkedPointers_WillPass_VisitsMultiple) {
   MockRawObjectVisitor visitor;

   static const constexpr int64_t kNumberOfPointers = 3;
   for(RawInt32 idx = 0; idx < kNumberOfPointers; idx++){
     auto ptr = Int32::TryAllocateIn(&semispace(), idx);
     ASSERT_TRUE(IsAllocated(ptr));
     ASSERT_TRUE(IsInt32(ptr));
     ASSERT_TRUE(Int32Eq(idx, ptr));
     ASSERT_NO_FATAL_FAILURE(Mark(ptr));
     EXPECT_CALL(visitor, Visit(IsPointerTo(ptr)));
   }

   ASSERT_TRUE(semispace().VisitMarkedPointers(&visitor));
 }
}