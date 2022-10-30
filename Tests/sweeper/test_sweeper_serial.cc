#include <gtest/gtest.h>

#include "helpers/assertions.h"
#include "helpers/alloc_helpers.h"
#include "poseidon/sweeper/sweeper_serial.h"

namespace poseidon {
#define UNALLOCATED 0 //TODO: cleanup

 using namespace ::testing;

 class SerialSweeperTest : public Test {
  protected:
   SerialSweeperTest() = default;
  public:
   ~SerialSweeperTest() override = default;
 };

 TEST_F(SerialSweeperTest, TestSweepPage_WillPass_SweepsNothing) {
   static const int64_t kOldZoneHeaderSize = OldZone::GetHeaderSize();
   static const int64_t kOldZoneSize = GetOldZoneSize();
   static const int64_t kTotalOldZoneSize = kOldZoneHeaderSize + kOldZoneSize;

   MemoryRegion region(kTotalOldZoneSize);
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto zone = OldZone::From(region);

   static constexpr const word kAValue = 33;
   auto a = TryAllocateMarkedWord(zone, kAValue);
   ASSERT_TRUE(IsOldWord(a, kAValue));
   ASSERT_TRUE(IsMarked(a));

   SerialSweeper sweeper;
   ASSERT_NO_FATAL_FAILURE(sweeper.Sweep(zone));
 }

 TEST_F(SerialSweeperTest, TestSweepPage_WillPass_SweepsOneObject) {
   NOT_IMPLEMENTED(ERROR); //TODO: implement
//   MemoryRegion region(GetOldPageSize());
//   FreeList free_list(region);
//   OldPage page(0, region);
//
//   FreeListPrinter::Print(free_list);
//   auto p1 = TryAllocateWord(free_list, 33);
//   ASSERT_TRUE(IsAllocated(p1));
//   ASSERT_FALSE(IsNew(p1));
//   ASSERT_TRUE(IsOld(p1));
//   ASSERT_FALSE(IsMarked(p1));
//   ASSERT_FALSE(IsRemembered(p1));
//   ASSERT_FALSE(IsForwarding(p1));
//
//   FreeListPrinter::Print(free_list);
//   SerialSweeper sweeper(&free_list);
//   ASSERT_TRUE(sweeper.SweepPage(&page));
//   FreeListPrinter::Print(free_list);
//
//   MockFreeListNodeVisitor visitor;
//   EXPECT_CALL(visitor, Visit)
//       .Times(2);
//   ASSERT_TRUE(free_list.VisitFreeNodes(&visitor));
 }

 TEST_F(SerialSweeperTest, TestSweepPage_WillPass_SweepsMultipleContiguousObjects) {
   NOT_IMPLEMENTED(ERROR); //TODO: implement
//   MemoryRegion region(GetOldPageSize());
//   FreeList free_list(region);
//   OldPage page(0, region);
//
//   static const constexpr int64_t kAValue = 33;
//   auto a = TryAllocateWord(free_list, kAValue);
//   ASSERT_TRUE(IsAllocated(a));
//   ASSERT_FALSE(IsNew(a));
//   ASSERT_TRUE(IsOld(a));
//   ASSERT_FALSE(IsMarked(a));
//   ASSERT_FALSE(IsRemembered(a));
//   ASSERT_FALSE(IsForwarding(a));
//
//   static constexpr const int64_t kBValue = 55;
//   auto b = TryAllocateWord(free_list, kBValue);
//   ASSERT_TRUE(IsAllocated(b));
//   ASSERT_FALSE(IsNew(b));
//   ASSERT_TRUE(IsOld(b));
//   ASSERT_FALSE(IsMarked(b));
//   ASSERT_FALSE(IsRemembered(b));
//   ASSERT_FALSE(IsForwarding(b));
//
//   static constexpr const int64_t kCValue = 99;
//   auto c = TryAllocateWord(free_list, kCValue);
//   ASSERT_TRUE(IsAllocated(c));
//   ASSERT_FALSE(IsNew(c));
//   ASSERT_TRUE(IsOld(c));
//   ASSERT_FALSE(IsMarked(c));
//   ASSERT_FALSE(IsRemembered(c));
//   ASSERT_FALSE(IsForwarding(c));
//
//   SerialSweeper sweeper(&free_list);
//   ASSERT_TRUE(sweeper.SweepPage(&page));
//
//   MockFreeListNodeVisitor visitor;
//   EXPECT_CALL(visitor, Visit)
//       .Times(4);
//   ASSERT_TRUE(free_list.VisitFreeNodes(&visitor));
 }

 TEST_F(SerialSweeperTest, TestSweepPage_WillPass_SweepsMultipleObjects) {
   NOT_IMPLEMENTED(ERROR); //TODO: implement
//   MemoryRegion region(GetOldPageSize());
//   FreeList free_list(region);
//   OldPage page(0, region);
//
//   static const constexpr int64_t kAValue = 33;
//   auto a = TryAllocateWord(free_list, kAValue);
//   ASSERT_TRUE(IsAllocated(a));
//   ASSERT_FALSE(IsNew(a));
//   ASSERT_TRUE(IsOld(a));
//   ASSERT_FALSE(IsMarked(a));
//   ASSERT_FALSE(IsRemembered(a));
//   ASSERT_FALSE(IsForwarding(a));
//
//   static constexpr const int64_t kBValue = 55;
//   auto b = TryAllocateMarkedWord(free_list, kBValue);
//   ASSERT_TRUE(IsAllocated(b));
//   ASSERT_FALSE(IsNew(b));
//   ASSERT_TRUE(IsOld(b));
//   ASSERT_TRUE(IsMarked(b));
//   ASSERT_FALSE(IsRemembered(b));
//   ASSERT_FALSE(IsForwarding(b));
//
//   static constexpr const int64_t kCValue = 99;
//   auto c = TryAllocateWord(free_list, kCValue);
//   ASSERT_TRUE(IsAllocated(c));
//   ASSERT_FALSE(IsNew(c));
//   ASSERT_TRUE(IsOld(c));
//   ASSERT_FALSE(IsMarked(c));
//   ASSERT_FALSE(IsRemembered(c));
//   ASSERT_FALSE(IsForwarding(c));
//
//   SerialSweeper sweeper(&free_list);
//   ASSERT_TRUE(sweeper.SweepPage(&page));
//
//   MockFreeListNodeVisitor visitor;
//   EXPECT_CALL(visitor, Visit)
//       .Times(3);
//   ASSERT_TRUE(free_list.VisitFreeNodes(&visitor));
//
//   ASSERT_TRUE(IsAllocated(a));
//   ASSERT_FALSE(IsWord(a, kAValue));
//
//   ASSERT_TRUE(IsAllocated(b));
//   ASSERT_FALSE(IsNew(b));
//   ASSERT_TRUE(IsOld(b));
//   ASSERT_TRUE(IsMarked(b));
//   ASSERT_FALSE(IsRemembered(b));
//   ASSERT_FALSE(IsForwarding(b));
//   ASSERT_TRUE(IsWord(b, kBValue));
//
//   ASSERT_TRUE(IsAllocated(a));
//   ASSERT_FALSE(IsWord(a, kCValue));
 }
}