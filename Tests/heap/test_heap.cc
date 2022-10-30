#include <gtest/gtest.h>

#include "poseidon/heap/heap.h"

namespace poseidon {
 using namespace ::testing;

 class HeapTest : public Test {
  public:
   HeapTest() = default;
   ~HeapTest() override = default;
 };

 TEST_F(HeapTest, TestFrom_WillFail_SizeLessThanZero) {

 }

 TEST_F(HeapTest, TestFrom_WillFail_SizeEqualToZero) {

 }

 //TODO: add test for checking other size constraints of the heap to ensure its fully divisible

 TEST_F(HeapTest, TestFrom_WillPass) {
   static const int64_t kNewZoneHeaderSize = NewZone::GetHeaderSize();
   static const int64_t kNewZoneSize = GetNewZoneSize();
   static const int64_t kNewZoneTotalSize = kNewZoneHeaderSize + kNewZoneSize;

   static const int64_t kOldZoneHeaderSize = OldZone::GetHeaderSize();
   static const int64_t kOldZoneSize = GetOldZoneSize();
   static const int64_t kOldZoneTotalSize = kOldZoneHeaderSize + kOldZoneSize;

   static const int64_t kHeapHeaderSize = Heap::GetHeaderSize();
   static const int64_t kHeapSize = kNewZoneTotalSize + kOldZoneTotalSize;
   static const int64_t kHeapTotalSize = kHeapHeaderSize + kHeapSize;

   MemoryRegion region(kHeapTotalSize);
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));

   auto heap = Heap::From(region);
   ASSERT_EQ(heap->GetHeapStartingAddress(), region.GetStartingAddress());
   ASSERT_EQ(heap->GetStartingAddress(), region.GetStartingAddress() + kHeapHeaderSize);
   ASSERT_EQ(heap->GetTotalSize(), region.GetSize());

   auto new_zone = heap->new_zone();
   ASSERT_EQ(new_zone->GetZoneStartingAddress(), heap->GetStartingAddress());
   ASSERT_EQ(new_zone->GetSize(), kNewZoneSize);

   auto old_zone = heap->old_zone();
   ASSERT_EQ(old_zone->GetZoneStartingAddress(), heap->GetStartingAddress() + kNewZoneTotalSize);
   ASSERT_EQ(old_zone->GetSize(), kOldZoneSize);
 }
}