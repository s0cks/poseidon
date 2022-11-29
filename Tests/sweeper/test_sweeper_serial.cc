#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "helpers.h"
#include "matchers/is_pointer_to.h"
#include "helpers/assertions.h"
#include "poseidon/heap/heap.h"
#include "sweeper/mock_sweeper.h"
#include "helpers/alloc_helpers.h"
#include "poseidon/sweeper/sweeper_serial.h"

namespace poseidon {
 using namespace ::testing;

 class SerialSweeperTest : public Test {
  protected:
   SerialSweeperTest() = default;

   static inline bool
   SerialSweep(Sweeper* sweeper, Heap* heap) {
     SerialSweeper serial_sweeper(sweeper);
     return serial_sweeper.Sweep();
   }
  public:
   ~SerialSweeperTest() override = default;
 };

 TEST_F(SerialSweeperTest, TestSweep_WillPass_SweepsNothing) {


   MemoryRegion region(GetTotalInitialHeapSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto heap = Heap::From(region);
   auto zone = heap->old_zone();

   static constexpr const word kAValue = 33;
   auto a = TryAllocateMarkedWord(zone, kAValue);
   ASSERT_TRUE(IsOldWord(a, kAValue));
   ASSERT_TRUE(IsMarked(a));

   MockSweeper sweeper(zone);
   ASSERT_TRUE(SerialSweep(&sweeper, heap));

   EXPECT_CALL(sweeper, Sweep)
    .Times(0);

   for(auto it = zone->pages_begin(); it != zone->pages_end(); it++)
     ASSERT_FALSE(zone->IsMarked(it));
 }

 TEST_F(SerialSweeperTest, TestSweepPage_WillPass_SweepsOneObject) {
   MemoryRegion region(GetTotalInitialHeapSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto heap = Heap::From(region);
   auto zone = heap->old_zone();

   static constexpr const word kAValue = 33;
   auto a = TryAllocateWord(zone, kAValue);
   ASSERT_TRUE(IsOldWord(a, kAValue));
   ASSERT_FALSE(IsMarked(a));

   MockSweeper sweeper(zone);
   EXPECT_CALL(sweeper, Sweep(IsPointerTo(a)))
    .Times(1)
    .WillOnce([](Pointer* ptr) {
      DLOG(INFO) << "sweeping " << (*ptr);
      ptr->SetFreeBit();
      memset((void*) ptr->GetObjectPointerAddress(), 0, ptr->GetSize());
      return true;
    });
   ASSERT_TRUE(SerialSweep(&sweeper, heap));

   ASSERT_TRUE(IsFree(a));

   for(auto it = zone->pages_begin(); it != zone->pages_end(); it++)
     ASSERT_FALSE(zone->IsMarked(it));
 }

 TEST_F(SerialSweeperTest, TestSweepPage_WillPass_SweepsMultipleContiguousObjects) {
   MemoryRegion region(GetTotalInitialHeapSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto heap = Heap::From(region);
   auto zone = heap->old_zone();

   static constexpr const word kAValue = 33;
   auto a = TryAllocateWord(zone, kAValue);
   ASSERT_TRUE(IsOldWord(a, kAValue));
   ASSERT_FALSE(IsMarked(a));

   static constexpr const word kBValue = 66;
   auto b = TryAllocateWord(zone, kBValue);
   ASSERT_TRUE(IsOldWord(b, kBValue));
   ASSERT_FALSE(IsMarked(b));

   static constexpr const word kCValue = 99;
   auto c = TryAllocateWord(zone, kCValue);
   ASSERT_TRUE(IsOldWord(c, kCValue));
   ASSERT_FALSE(IsMarked(c));

   MockSweeper sweeper(zone);
   EXPECT_CALL(sweeper, Sweep(IsPointerTo(a)))
    .Times(1)
    .WillOnce([](Pointer* ptr) {
      DLOG(INFO) << "sweeping " << (*ptr);
      ptr->SetFreeBit();
      memset((void*) ptr->GetObjectPointerAddress(), 0, ptr->tag().GetSize());
      return true;
    });
   EXPECT_CALL(sweeper, Sweep(IsPointerTo(b)))
    .Times(1)
    .WillOnce([](Pointer* ptr) {
      DLOG(INFO) << "sweeping " << (*ptr);
      ptr->SetFreeBit();
      memset((void*) ptr->GetObjectPointerAddress(), 0, ptr->tag().GetSize());
      return true;
    });
   EXPECT_CALL(sweeper, Sweep(IsPointerTo(c)))
    .Times(1)
    .WillOnce([](Pointer* ptr) {
      DLOG(INFO) << "sweeping " << (*ptr);
      ptr->SetFreeBit();
      memset((void*) ptr->GetObjectPointerAddress(), 0, ptr->tag().GetSize());
      return true;
    });
   ASSERT_TRUE(SerialSweep(&sweeper, heap));

   ASSERT_TRUE(IsFree(a));
   ASSERT_TRUE(IsFree(b));
   ASSERT_TRUE(IsFree(c));

   for(auto it = zone->pages_begin(); it != zone->pages_end(); it++)
     ASSERT_FALSE(zone->IsMarked(it));
 }

 TEST_F(SerialSweeperTest, TestSweepPage_WillPass_SweepsMultipleNoncontiguousObjects) {
   MemoryRegion region(GetTotalInitialHeapSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto heap = Heap::From(region);
   auto zone = heap->old_zone();

   static constexpr const word kAValue = 33;
   auto a = TryAllocateWord(zone, kAValue);
   ASSERT_TRUE(IsOldWord(a, kAValue));
   ASSERT_FALSE(IsMarked(a));

   static constexpr const word kBValue = 66;
   auto b = TryAllocateMarkedWord(zone, kBValue);
   ASSERT_TRUE(IsOldWord(b, kBValue));
   ASSERT_TRUE(IsMarked(b));

   static constexpr const word kCValue = 99;
   auto c = TryAllocateWord(zone, kCValue);
   ASSERT_TRUE(IsOldWord(c, kCValue));
   ASSERT_FALSE(IsMarked(c));

   MockSweeper sweeper(zone);
   EXPECT_CALL(sweeper, Sweep(IsPointerTo(a)))
    .Times(1)
    .WillOnce([](Pointer* ptr) {
      DLOG(INFO) << "sweeping " << (*ptr);
      ptr->SetFreeBit();
      memset((void*) ptr->GetObjectPointerAddress(), 0, ptr->tag().GetSize());
      return true;
    });
   EXPECT_CALL(sweeper, Sweep(IsPointerTo(b)))
    .Times(0);
   EXPECT_CALL(sweeper, Sweep(IsPointerTo(c)))
    .Times(1)
    .WillOnce([](Pointer* ptr) {
      DLOG(INFO) << "sweeping " << (*ptr);
      ptr->SetFreeBit();
      memset((void*) ptr->GetObjectPointerAddress(), 0, ptr->tag().GetSize());
      return true;
    });
   ASSERT_TRUE(SerialSweep(&sweeper, heap));

   ASSERT_TRUE(IsFree(a));

   ASSERT_FALSE(IsFree(b));
   ASSERT_TRUE(IsOldWord(b, kBValue));
   ASSERT_TRUE(IsMarked(b));

   ASSERT_TRUE(IsFree(c));

   for(auto it = zone->pages_begin(); it != zone->pages_end(); it++)
     ASSERT_FALSE(zone->IsMarked(it));
 }
}