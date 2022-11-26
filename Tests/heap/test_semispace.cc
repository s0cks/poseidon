#include "helpers/assertions.h"
#include "helpers/alloc_helpers.h"
#include "heap/semispace_printer.h"
#include "mock_raw_object_visitor.h"

#include "poseidon/type/int.h"
#include "assertions/int_assertions.h"

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
   MemoryRegion region(GetNewZoneSemispaceSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   Semispace semispace(region);
   ASSERT_EQ(semispace.GetStartingAddress(), region.GetStartingAddress());
   ASSERT_TRUE(semispace.IsEmpty());
   ASSERT_EQ(semispace.GetCurrentAddress(), semispace.GetCurrentAddress());
   ASSERT_EQ(semispace.GetSize(), region.GetSize());
 }

 TEST_F(SemispaceTest, TestSwap_WillPass){
   // a == r1
   MemoryRegion r1(GetNewZoneSemispaceSize());
   ASSERT_TRUE(r1.Protect(MemoryRegion::kReadWrite));
   Semispace a(r1);
   ASSERT_EQ(a.GetStartingAddress(), r1.GetStartingAddress());
   ASSERT_EQ(a.GetCurrentAddress(), r1.GetStartingAddress());
   ASSERT_EQ(a.GetSize(), r1.GetSize());

   static constexpr const word kAValue = 1034;
   auto ptr = Int::TryAllocateIn<>(&a, kAValue);
   ASSERT_NE(ptr, nullptr);
   ASSERT_TRUE(IsInt(ptr->raw_ptr()));
   ASSERT_TRUE(IntEq(kAValue, ptr));
   ASSERT_TRUE(a.Intersects(*ptr->raw_ptr()));

   // b == r2
   MemoryRegion r2(GetNewZoneSemispaceSize());
   ASSERT_TRUE(r2.Protect(MemoryRegion::kReadWrite));
   Semispace b(r2);
   ASSERT_EQ(b.GetStartingAddress(), r2.GetStartingAddress());
   ASSERT_EQ(b.GetCurrentAddress(), r2.GetStartingAddress());
   ASSERT_EQ(b.GetSize(), r2.GetSize());
   ASSERT_FALSE(b.Intersects(*ptr->raw_ptr()));

   ASSERT_NE(a, b);
   ASSERT_NO_FATAL_FAILURE(std::swap(a, b));
   ASSERT_TRUE(IsInt(ptr->raw_ptr()));
   ASSERT_TRUE(IntEq(kAValue, ptr));

   // a == r2
   ASSERT_EQ(a.GetStartingAddress(), r2.GetStartingAddress());
   ASSERT_EQ(a.GetCurrentAddress(), r2.GetStartingAddress());
   ASSERT_EQ(a.GetSize(), r2.GetSize());
   ASSERT_FALSE(a.Intersects(*ptr->raw_ptr()));

   // b == r1
   ASSERT_EQ(b.GetStartingAddress(), r1.GetStartingAddress());
   ASSERT_EQ(b.GetCurrentAddress(), r1.GetStartingAddress() + ptr->raw_ptr()->GetTotalSize());
   ASSERT_EQ(b.GetSize(), r1.GetSize());
   ASSERT_TRUE(b.Intersects(*ptr->raw_ptr()));
 }

#define DEFINE_TRY_ALLOCATE_BYTES_FAILS_TEST(TestSuite, TestName, Zone, ZoneSize, NumberOfBytes) \
  TEST_F(TestSuite, TestTryAllocateBytes_WillFail_##TestName) {                                  \
    MemoryRegion region(ZoneSize);                                                               \
    ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));                                       \
    Zone zone(region);                                                                           \
    auto new_ptr = zone.TryAllocateBytes(NumberOfBytes);                                         \
    ASSERT_EQ(new_ptr, UNALLOCATED);                                                             \
  }

#define DEFINE_TRY_AlLOCATE_BYTES_FAILS_SEMISPACE_TEST(TestName, NumberOfBytes) \
  DEFINE_TRY_ALLOCATE_BYTES_FAILS_TEST(SemispaceTest, TestName, Semispace, GetNewZoneSemispaceSize(), NumberOfBytes)

 DEFINE_TRY_AlLOCATE_BYTES_FAILS_SEMISPACE_TEST(SizeLessThanZero, -1);
 DEFINE_TRY_AlLOCATE_BYTES_FAILS_SEMISPACE_TEST(SizeEqualsZero, 0);
 DEFINE_TRY_AlLOCATE_BYTES_FAILS_SEMISPACE_TEST(SizeLessThanMin, Semispace::GetMinimumObjectSize() - 1);
 DEFINE_TRY_AlLOCATE_BYTES_FAILS_SEMISPACE_TEST(SizeGreaterThanMax, Semispace::GetMaximumObjectSize() + 1);
 DEFINE_TRY_AlLOCATE_BYTES_FAILS_SEMISPACE_TEST(SizeEqualToSemispaceSize, GetNewZoneSemispaceSize());
 DEFINE_TRY_AlLOCATE_BYTES_FAILS_SEMISPACE_TEST(SizeGreaterThanSemispaceSize, GetNewZoneSemispaceSize() + 1);

 TEST_F(SemispaceTest, TestTryAllocateBytes_WillFail_NotEnoughSpace){
   auto semispace = Semispace(GetNewZoneSemispaceSize());
   ASSERT_NO_FATAL_FAILURE(semispace.SetWritable());
   SetCurrentAddress(semispace, semispace.GetEndingAddress());
   auto new_ptr = semispace.TryAllocateBytes(kWordSize);
   ASSERT_EQ(new_ptr, UNALLOCATED);
 }

 TEST_F(SemispaceTest, TestTryAllocate){
   auto semispace = Semispace(GetNewZoneSemispaceSize());
   ASSERT_NO_FATAL_FAILURE(semispace.SetWritable());

   static const constexpr RawInt kDefaultValue = 42;
   auto ptr = Int::TryAllocateIn<>(&semispace, kDefaultValue);
   ASSERT_NE(ptr, nullptr);
   ASSERT_TRUE(IsAllocated(ptr->raw_ptr()));
   ASSERT_TRUE(IsNew(ptr->raw_ptr()));
   ASSERT_FALSE(IsOld(ptr->raw_ptr()));
   ASSERT_FALSE(IsMarked(ptr->raw_ptr()));
   ASSERT_FALSE(IsRemembered(ptr->raw_ptr()));
   ASSERT_FALSE(IsForwarding(ptr->raw_ptr()));
   ASSERT_TRUE(IsInt(ptr->raw_ptr()));
   ASSERT_TRUE(IntEq(kDefaultValue, ptr));

   ASSERT_TRUE(SemispacePrinter<>::PrintSemispace(semispace));
 }

 TEST_F(SemispaceTest, TestVisitPointers){
   auto semispace = Semispace(GetNewZoneSemispaceSize());
   ASSERT_NO_FATAL_FAILURE(semispace.SetWritable());

   static const constexpr int64_t kNumberOfPointers = 3;
   for(RawInt idx = 0; idx < kNumberOfPointers; idx++){
     auto ptr = Int::TryAllocateIn(&semispace, idx);
     ASSERT_NE(ptr, nullptr);
     ASSERT_TRUE(IsAllocated(ptr->raw_ptr()));
     ASSERT_TRUE(IsNew(ptr->raw_ptr()));
     ASSERT_FALSE(IsOld(ptr->raw_ptr()));
     ASSERT_FALSE(IsMarked(ptr->raw_ptr()));
     ASSERT_FALSE(IsRemembered(ptr->raw_ptr()));
     ASSERT_FALSE(IsForwarding(ptr->raw_ptr()));
     ASSERT_TRUE(IsInt(ptr->raw_ptr()));
     ASSERT_TRUE(IntEq(idx, ptr));
   }

   MockRawObjectVisitor visitor;
   EXPECT_CALL(visitor, Visit)
     .Times(kNumberOfPointers);
   ASSERT_NO_FATAL_FAILURE(semispace.VisitPointers(&visitor));

   ASSERT_NO_FATAL_FAILURE(SemispacePrinter<>::PrintSemispace(semispace));
 }

 TEST_F(SemispaceTest, TestVisitMarkedPointers){
   auto semispace = Semispace(GetNewZoneSemispaceSize());
   ASSERT_NO_FATAL_FAILURE(semispace.SetWritable());

   static const constexpr int64_t kNumberOfUnmarkedPointers = 1;
   static const constexpr int64_t kNumberOfMarkedPointers = 3;

   for(RawInt idx = 0; idx < kNumberOfUnmarkedPointers; idx++){
     auto ptr = Int::TryAllocateIn(&semispace, idx);
     ASSERT_NE(ptr, nullptr);
     ASSERT_TRUE(IsAllocated(ptr->raw_ptr()));
     ASSERT_TRUE(IsNew(ptr->raw_ptr()));
     ASSERT_FALSE(IsOld(ptr->raw_ptr()));
     ASSERT_FALSE(IsMarked(ptr->raw_ptr()));
     ASSERT_FALSE(IsRemembered(ptr->raw_ptr()));
     ASSERT_FALSE(IsForwarding(ptr->raw_ptr()));
     ASSERT_TRUE(IsInt(ptr->raw_ptr()));
     ASSERT_TRUE(IntEq(idx, ptr));
   }

   for(RawInt idx = 0; idx < kNumberOfMarkedPointers; idx++){
     auto ptr = Int::TryAllocateIn(&semispace, idx);
     ptr->raw_ptr()->SetMarkedBit(); //TODO: cleanup

     ASSERT_NE(ptr, nullptr);
     ASSERT_TRUE(IsAllocated(ptr->raw_ptr()));
     ASSERT_TRUE(IsNew(ptr->raw_ptr()));
     ASSERT_FALSE(IsOld(ptr->raw_ptr()));
     ASSERT_TRUE(IsMarked(ptr->raw_ptr()));
     ASSERT_FALSE(IsRemembered(ptr->raw_ptr()));
     ASSERT_FALSE(IsForwarding(ptr->raw_ptr()));
     ASSERT_TRUE(IsInt(ptr->raw_ptr()));
     ASSERT_TRUE(IntEq(idx, ptr));
   }

   MockRawObjectVisitor visitor;
   EXPECT_CALL(visitor, Visit)
       .Times(kNumberOfMarkedPointers);
   ASSERT_NO_FATAL_FAILURE(semispace.VisitMarkedPointers(&visitor));

   ASSERT_NO_FATAL_FAILURE(SemispacePrinter<>::PrintSemispace(semispace));
 }
}