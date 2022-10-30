#include <gtest/gtest.h>

#include "poseidon/heap/heap.h"
#include "helpers/assertions.h"
#include "helpers/alloc_helpers.h"

namespace poseidon {
 using namespace ::testing;

 class HeapTest : public Test {
  public:
   HeapTest() = default;
   ~HeapTest() override = default;
 };

 static const int64_t kNewZoneHeaderSize = NewZone::GetHeaderSize();
 static const int64_t kNewZoneSize = GetNewZoneSize();
 static const int64_t kNewZoneTotalSize = kNewZoneHeaderSize + kNewZoneSize;

 static const int64_t kOldZoneHeaderSize = OldZone::GetHeaderSize();
 static const int64_t kOldZoneSize = GetOldZoneSize();
 static const int64_t kOldZoneTotalSize = kOldZoneHeaderSize + kOldZoneSize;

 static const int64_t kHeapHeaderSize = Heap::GetHeaderSize();
 static const int64_t kHeapSize = kNewZoneTotalSize + kOldZoneTotalSize;
 static const int64_t kHeapTotalSize = kHeapHeaderSize + kHeapSize;

 TEST_F(HeapTest, TestFrom_WillFail_SizeLessThanZero) {
   MemoryRegion region(kHeapTotalSize);
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto heap = Heap::From(MemoryRegion(region.GetStartingAddress(), -1));
   ASSERT_EQ(heap, nullptr);
 }

 TEST_F(HeapTest, TestFrom_WillFail_SizeEqualToZero) {
   MemoryRegion region(kHeapTotalSize);
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto heap = Heap::From(MemoryRegion(region.GetStartingAddress(), 0));
   ASSERT_EQ(heap, nullptr);
 }

 //TODO: add test for checking other size constraints of the heap to ensure its fully divisible

 TEST_F(HeapTest, TestFrom_WillPass) {
   MemoryRegion region(kHeapTotalSize);
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));

   auto heap = Heap::From(region);
   ASSERT_NE(heap, nullptr);
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

 TEST_F(HeapTest, TestTryAllocate_WillFail_SizeLessThanZero) {
   MemoryRegion region(kHeapTotalSize);
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));

   auto heap = Heap::From(region);
   ASSERT_NE(heap, nullptr);

   auto ptr = TryAllocateBytes<Heap>(heap, -1);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(HeapTest, TestTryAllocate_WillFail_SizeEqualToZero) {
   MemoryRegion region(kHeapTotalSize);
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));

   auto heap = Heap::From(region);
   ASSERT_NE(heap, nullptr);

   auto ptr = TryAllocateBytes<Heap>(heap, 0);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(HeapTest, TestTryAllocate_WillFail_SizeLessThanWordSize) {
   MemoryRegion region(kHeapTotalSize);
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));

   auto heap = Heap::From(region);
   ASSERT_NE(heap, nullptr);

   auto ptr = TryAllocateBytes<Heap>(heap, kWordSize - 1);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(HeapTest, TestTryAllocate_WillFail_SizeGreaterOldPageSize) {
   MemoryRegion region(kHeapTotalSize);
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));

   auto heap = Heap::From(region);
   ASSERT_NE(heap, nullptr);

   auto ptr = TryAllocateBytes<Heap>(heap, GetOldPageSize() + 1);
   ASSERT_EQ(ptr, UNALLOCATED);
 }

 TEST_F(HeapTest, TestTryAllocate_WillPass_NewPointer) {
   MemoryRegion region(kHeapTotalSize);
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));

   auto heap = Heap::From(region);
   ASSERT_NE(heap, nullptr);

   static constexpr const word kValue = 33;
   auto ptr = TryAllocateWord<Heap>(heap, kValue);
   ASSERT_TRUE(IsAllocated(ptr));
   ASSERT_TRUE(IsNewWord(ptr, kValue));
   ASSERT_TRUE(heap->new_zone()->Contains(*ptr));
   ASSERT_TRUE(heap->new_zone()->GetPageAt(0)->Contains(*ptr));
   ASSERT_TRUE(heap->new_zone()->fromspace().Contains(*ptr));
 }

 TEST_F(HeapTest, TestTryAllocate_WillPass_LargePointer) {
   MemoryRegion region(kHeapTotalSize);
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));

   auto heap = Heap::From(region);
   ASSERT_NE(heap, nullptr);

   const auto kPointerSize = GetNewPageSize();
   auto ptr = TryAllocateBytes<Heap>(heap, kPointerSize);
   ASSERT_NE(ptr, UNALLOCATED);
   ASSERT_TRUE(heap->old_zone()->Contains(ptr));
   ASSERT_TRUE(heap->old_zone()->GetPageAt(0)->Contains(ptr));
 }
}