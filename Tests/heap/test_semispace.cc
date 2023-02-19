#include <gtest/gtest.h>

#include "poseidon/object.h"
#include "poseidon/heap/semispace.h"

#include "helpers.h"

#include "poseidon/marker/marker.h"

#include "assertions/ptr_assertions.h"
#include "assertions/type_assertions.h"

#include "matchers/is_pointer_to.h"
#include "mock_raw_object_visitor.h"

namespace poseidon{
 using namespace ::testing;

 class SemispaceTest : public Test{
  protected:
   MemoryRegion region_;
   Semispace semispace_;
   Marker marker_;

   SemispaceTest():
    Test(),
    region_(flags::GetNewZoneSemispaceSize(), MemoryRegion::kReadOnly),
    semispace_(Space::kFromSpace, region_),
    marker_() {
   }

   inline MemoryRegion& region() {
     return region_;
   }

   inline Semispace& semispace() {
     return semispace_;
   }

   static inline void
   SetCurrentAddress(Semispace& semispace, const uword address) {
     ASSERT_TRUE(semispace.Contains(address));
     semispace.current_ = address;
   }

   inline Marker& marker() {
     return marker_;
   }

   inline void Mark(Pointer* ptr) {
     LOG_IF(FATAL, !marker().Visit(ptr)) << "failed to mark " << (*ptr);
   }

   template<class T>
   inline void Mark(T* value) {
     return Mark(value->raw_ptr());
   }
  public:
   ~SemispaceTest() override = default;

   void SetUp() override {
     ASSERT_NO_FATAL_FAILURE(semispace().SetWritable());
     ASSERT_NO_FATAL_FAILURE(semispace().Clear());
     ASSERT_NO_FATAL_FAILURE(semispace().SetReadOnly());
     SemispacePrinter::Print(&semispace());
     ASSERT_NO_FATAL_FAILURE(semispace().SetWritable());
   }

   void TearDown() override {
     ASSERT_NO_FATAL_FAILURE(semispace().SetReadOnly());
     SemispacePrinter::Print(&semispace());
   }
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

#define DEFINE_TRY_ALLOCATE_BYTES_FAILS_SEMISPACE_TEST(TestName, NumberOfBytes) \
  DEFINE_TRY_ALLOCATE_BYTES_FAILS_TEST(SemispaceTest, TestName, Semispace, flags::GetNewZoneSemispaceSize(), NumberOfBytes)

 DEFINE_TRY_ALLOCATE_BYTES_FAILS_SEMISPACE_TEST(SizeLessThanZero, -1);
 DEFINE_TRY_ALLOCATE_BYTES_FAILS_SEMISPACE_TEST(SizeEqualsZero, 0);
 DEFINE_TRY_ALLOCATE_BYTES_FAILS_SEMISPACE_TEST(SizeLessThanMin, Semispace::GetMinimumObjectSize() - 1);
 DEFINE_TRY_ALLOCATE_BYTES_FAILS_SEMISPACE_TEST(SizeGreaterThanMax, Semispace::GetMaximumObjectSize() + 1);
 DEFINE_TRY_ALLOCATE_BYTES_FAILS_SEMISPACE_TEST(SizeEqualToSemispaceSize, flags::GetNewZoneSemispaceSize());
 DEFINE_TRY_ALLOCATE_BYTES_FAILS_SEMISPACE_TEST(SizeGreaterThanSemispaceSize, flags::GetNewZoneSemispaceSize() + 1);

 TEST_F(SemispaceTest, TestTryAllocateBytes_WillFail_NotEnoughSpace){
   SetCurrentAddress(semispace(), semispace().GetEndingAddress());
   auto new_ptr = semispace().TryAllocateBytes(kWordSize);
   ASSERT_TRUE(IsUnallocated(new_ptr));
 }

#define DEFINE_TRY_ALLOCATE_TYPE_PASSES_SEMISPACE_TEST(TestName, Type) \
 TEST_F(SemispaceTest, TestTryAllocate_WillPass_##Type){               \
   static const constexpr RawInt32 kDefaultValue = 42;                 \
   auto ptr = Type::TryAllocateIn<>(&semispace(), kDefaultValue);      \
   ASSERT_TRUE(IsAllocated(ptr));                                      \
   ASSERT_TRUE(IsNew(ptr));                                            \
   ASSERT_FALSE(IsOld(ptr));                                           \
   ASSERT_FALSE(IsMarked(ptr));                                        \
   ASSERT_FALSE(IsRemembered(ptr));                                    \
   ASSERT_FALSE(IsForwarding(ptr));                                    \
   ASSERT_TRUE(Is##Type(ptr));                                         \
   ASSERT_TRUE(Type##Eq(kDefaultValue, ptr));                          \
 }
 DEFINE_TRY_ALLOCATE_TYPE_PASSES_SEMISPACE_TEST(Int32, Int32);

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