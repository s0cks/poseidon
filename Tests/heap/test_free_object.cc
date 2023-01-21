#include <gtest/gtest.h>

#include "poseidon/heap/zone/old_zone.h"
#include "poseidon/freelist/free_ptr.h"

namespace poseidon {
#define UNALLOCATED 0 //TODO: cleanup

 using namespace ::testing;

 class FreeObjectTest : public Test {
  protected:
   FreeObjectTest() = default;
  public:
   ~FreeObjectTest() override = default;
 };

 TEST_F(FreeObjectTest, TestFrom_WillFail_StartingAddressEqualsZero) {
   MemoryRegion region;
   auto ptr = FreePointer::From(region);
   ASSERT_EQ(ptr, nullptr);
 }

 TEST_F(FreeObjectTest, TestFrom_WillFail_SizeEqualsZero) {
   MemoryRegion region(1 * kKB); // use a real region starting address
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto ptr = FreePointer::From(MemoryRegion(region.GetStartingAddress(), 0)); // but a zero for the size
   ASSERT_EQ(ptr, nullptr);
 }

 TEST_F(FreeObjectTest, TestFrom_WillFail_SizeGreaterThanOldZoneSize) {
   MemoryRegion region(flags::GetOldZoneSize() + 1);
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto ptr = FreePointer::From(region);
   ASSERT_EQ(ptr, nullptr);
 }

 TEST_F(FreeObjectTest, TestFrom_WillPass_EqualToWordSize) {
   MemoryRegion region(kWordSize + sizeof(Pointer));
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto ptr = FreePointer::From(region);
   ASSERT_EQ(ptr->GetStartingAddress(), region.GetStartingAddress());
   ASSERT_EQ(ptr->GetSize(), region.GetSize());
   ASSERT_EQ(ptr->GetEndingAddress(), region.GetEndingAddress());
   ASSERT_EQ(ptr->GetNextAddress(), UNALLOCATED);
   ASSERT_TRUE(ptr->IsFree());
   ASSERT_TRUE(ptr->IsOld());
 }

 TEST_F(FreeObjectTest, TestFrom_WillPass_EqualToOldZoneSize) {
   MemoryRegion region(flags::GetOldZoneSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto ptr = FreePointer::From(region);
   ASSERT_EQ(ptr->GetStartingAddress(), region.GetStartingAddress());
   ASSERT_EQ(ptr->GetSize(), region.GetSize());
   ASSERT_EQ(ptr->GetEndingAddress(), region.GetEndingAddress());
   ASSERT_EQ(ptr->GetNextAddress(), UNALLOCATED);
   ASSERT_TRUE(ptr->IsFree());
   ASSERT_TRUE(ptr->IsOld());
 }

 TEST_F(FreeObjectTest, TestEquals_WillPass) {
   MemoryRegion region(1 * kMB);
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto a = FreePointer::From(region);
   ASSERT_NE(a, nullptr);

   auto b = FreePointer::From(region);
   ASSERT_NE(b, nullptr);
   ASSERT_TRUE((*a) == (*b));
 }

 TEST_F(FreeObjectTest, TestEquals_WillFail_DifferentStartingAddresses) {
   MemoryRegion r1(1 * kKB);
   ASSERT_TRUE(r1.Protect(MemoryRegion::kReadWrite));
   auto a = FreePointer::From(r1);
   ASSERT_NE(a, nullptr);

   MemoryRegion r2(1 * kKB);
   ASSERT_TRUE(r2.Protect(MemoryRegion::kReadWrite));
   auto b = FreePointer::From(r2);
   ASSERT_NE(b, nullptr);

   ASSERT_FALSE((*a) == (*b));
 }

 TEST_F(FreeObjectTest, TestEquals_WillFail_DifferentSizes) {
   MemoryRegion region(512 * kKB);
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto a = FreePointer::From(MemoryRegion::Subregion(region, 0, 256 * kKB));
   ASSERT_NE(a, nullptr);

   auto b = FreePointer::From(MemoryRegion::Subregion(region, 256 * kKB, 256 * kKB));
   ASSERT_NE(b, nullptr);

   ASSERT_FALSE((*a) == (*b));
 }

 TEST_F(FreeObjectTest, TestNotEquals_WillPass) {
   MemoryRegion r1(1 * kKB);
   ASSERT_TRUE(r1.Protect(MemoryRegion::kReadWrite));
   auto a = FreePointer::From(r1);
   ASSERT_NE(a, nullptr);

   MemoryRegion r2(1 * kKB);
   ASSERT_TRUE(r2.Protect(MemoryRegion::kReadWrite));
   auto b = FreePointer::From(r2);
   ASSERT_NE(b, nullptr);

   ASSERT_TRUE((*a) != (*b));
 }
}