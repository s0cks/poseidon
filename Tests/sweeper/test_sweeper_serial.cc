#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "matchers/is_pointer_to.h"
#include "helpers/assertions.h"
#include "poseidon/heap/heap.h"
#include "sweeper/mock_sweeper.h"
#include "poseidon/type.h"
#include "helpers/alloc_helpers.h"
#include "assertions/type_assertions.h"
#include "poseidon/sweeper/sweeper_serial.h"

namespace poseidon {
 using namespace ::testing;

 class SerialSweeperTest : public Test {
  protected:
   MemoryRegion region_;
   OldZone zone_;

   SerialSweeperTest():
    region_(flags::GetOldZoneSize(), MemoryRegion::kReadWrite),
    zone_(region_) {
   }

   inline MemoryRegion& region() {
     return region_;
   }

   inline OldZone& zone() {
     return zone_;
   }

   inline bool
   SerialSweep(Sweeper* sweeper) {
     SerialSweeper serial_sweeper(sweeper, &zone());
     return serial_sweeper.Sweep();
   }
  public:
   ~SerialSweeperTest() override = default;

   void SetUp() override {
     ASSERT_TRUE(region().Protect(MemoryRegion::kReadWrite));
     ASSERT_NO_FATAL_FAILURE(zone().ClearOldZone());
   }

   void TearDown() override {
     ASSERT_TRUE(region().Protect(MemoryRegion::kReadOnly));
   }
 };

 TEST_F(SerialSweeperTest, TestSweep_WillPass_SweepsNothing) {
   static constexpr const RawInt32 kAValue = 33;
   auto a_ptr = Int32::TryAllocateIn(&zone(), kAValue);
   ASSERT_NE(a_ptr, nullptr);
   ASSERT_TRUE(IsInt32(a_ptr->raw_ptr()));
   ASSERT_TRUE(Int32Eq(kAValue, a_ptr));
   ASSERT_NO_FATAL_FAILURE(a_ptr->raw_ptr()->SetMarkedBit());
   ASSERT_TRUE(IsMarked(a_ptr->raw_ptr()));

   MockSweeper sweeper;
   ASSERT_TRUE(SerialSweep(&sweeper));

   EXPECT_CALL(sweeper, Sweep)
    .Times(0);
 }

 TEST_F(SerialSweeperTest, TestSweepPage_WillPass_SweepsOneObject_First) {
   static constexpr const RawInt32 kAValue = 33;
   auto a_ptr = Int32::TryAllocateIn(&zone(), kAValue);
   ASSERT_NE(a_ptr, nullptr);
   ASSERT_TRUE(IsInt32(a_ptr->raw_ptr()));
   ASSERT_TRUE(Int32Eq(kAValue, a_ptr));
   ASSERT_FALSE(IsMarked(a_ptr->raw_ptr()));

   static constexpr const RawInt32 kBValue = 99;
   auto b_ptr = Int32::TryAllocateIn(&zone(), kBValue);
   ASSERT_NE(b_ptr, nullptr);
   ASSERT_TRUE(IsInt32(b_ptr->raw_ptr()));
   ASSERT_TRUE(Int32Eq(kBValue, b_ptr));
   ASSERT_NO_FATAL_FAILURE(b_ptr->raw_ptr()->SetMarkedBit());
   ASSERT_TRUE(IsMarked(b_ptr->raw_ptr()));

   MockSweeper sweeper;
   EXPECT_CALL(sweeper, Sweep(IsPointerTo(a_ptr->raw_ptr())))
    .WillOnce([](Pointer* ptr) {
      DLOG(INFO) << "sweeping " << (*ptr);
      ptr->ClearMarkedBit();
      ptr->SetFreeBit();
      memset((void*) ptr->GetObjectPointerAddress(), 0, ptr->GetSize());
      return true;
    });
   ASSERT_TRUE(SerialSweep(&sweeper));

   //TODO: ASSERT_FALSE(IsInt32(a_ptr->raw_ptr()));
   ASSERT_FALSE(Int32Eq(kAValue, a_ptr));
   ASSERT_FALSE(IsMarked(a_ptr->raw_ptr()));
   ASSERT_TRUE(IsFree(a_ptr->raw_ptr()));

   ASSERT_TRUE(IsInt32(b_ptr->raw_ptr()));
   ASSERT_TRUE(Int32Eq(kBValue, b_ptr));
   ASSERT_TRUE(IsMarked(b_ptr->raw_ptr())); //TODO: should we unmark the object after sweeping?
 }

 TEST_F(SerialSweeperTest, TestSweepPage_WillPass_SweepsOneObject_Second) {
   static constexpr const RawInt32 kAValue = 33;
   auto a_ptr = Int32::TryAllocateIn(&zone(), kAValue);
   ASSERT_NE(a_ptr, nullptr);
   ASSERT_TRUE(IsInt32(a_ptr->raw_ptr()));
   ASSERT_TRUE(Int32Eq(kAValue, a_ptr));
   ASSERT_NO_FATAL_FAILURE(a_ptr->raw_ptr()->SetMarkedBit());
   ASSERT_TRUE(IsMarked(a_ptr->raw_ptr()));

   static constexpr const RawInt32 kBValue = 99;
   auto b_ptr = Int32::TryAllocateIn(&zone(), kBValue);
   ASSERT_NE(b_ptr, nullptr);
   ASSERT_TRUE(IsInt32(b_ptr->raw_ptr()));
   ASSERT_TRUE(Int32Eq(kBValue, b_ptr));
   ASSERT_FALSE(IsMarked(b_ptr->raw_ptr()));

   MockSweeper sweeper;
   EXPECT_CALL(sweeper, Sweep(IsPointerTo(b_ptr->raw_ptr())))
     .WillOnce([](Pointer* ptr) {
       DLOG(INFO) << "sweeping " << (*ptr);
       ptr->ClearMarkedBit();
       ptr->SetFreeBit();
       memset((void*) ptr->GetObjectPointerAddress(), 0, ptr->GetSize());
       return true;
     });
   ASSERT_TRUE(SerialSweep(&sweeper));

   ASSERT_TRUE(IsInt32(a_ptr->raw_ptr()));
   ASSERT_TRUE(Int32Eq(kAValue, a_ptr));
   ASSERT_TRUE(IsMarked(a_ptr->raw_ptr())); //TODO: should we unmark the object after sweeping?

   //TODO: ASSERT_FALSE(IsInt32(a_ptr->raw_ptr()));
   ASSERT_FALSE(Int32Eq(kBValue, b_ptr));
   ASSERT_FALSE(IsMarked(b_ptr->raw_ptr()));
   ASSERT_TRUE(IsFree(b_ptr->raw_ptr()));
 }

 TEST_F(SerialSweeperTest, TestSweepPage_WillPass_SweepsMultipleContiguousObjects) {
   static constexpr const RawInt32 kAValue = 33;
   auto a_ptr = Int32::TryAllocateIn(&zone(), kAValue);
   ASSERT_NE(a_ptr, nullptr);
   ASSERT_TRUE(IsInt32(a_ptr->raw_ptr()));
   ASSERT_TRUE(Int32Eq(kAValue, a_ptr));
   ASSERT_FALSE(IsMarked(a_ptr->raw_ptr()));

   static constexpr const RawInt32 kBValue = 99;
   auto b_ptr = Int32::TryAllocateIn(&zone(), kBValue);
   ASSERT_NE(b_ptr, nullptr);
   ASSERT_TRUE(IsInt32(b_ptr->raw_ptr()));
   ASSERT_TRUE(Int32Eq(kBValue, b_ptr));
   ASSERT_FALSE(IsMarked(b_ptr->raw_ptr()));

   MockSweeper sweeper;
   EXPECT_CALL(sweeper, Sweep(IsPointerTo(a_ptr->raw_ptr())))
     .WillOnce([](Pointer* ptr) {
       DLOG(INFO) << "sweeping " << (*ptr);
       ptr->ClearMarkedBit();
       ptr->SetFreeBit();
       memset((void*) ptr->GetObjectPointerAddress(), 0, ptr->GetSize());
       return true;
     });
   EXPECT_CALL(sweeper, Sweep(IsPointerTo(b_ptr->raw_ptr())))
     .WillOnce([](Pointer* ptr) {
       DLOG(INFO) << "sweeping " << (*ptr);
       ptr->ClearMarkedBit();
       ptr->SetFreeBit();
       memset((void*) ptr->GetObjectPointerAddress(), 0, ptr->GetSize());
       return true;
     });
   ASSERT_TRUE(SerialSweep(&sweeper));

   //TODO: ASSERT_FALSE(IsInt32(a_ptr->raw_ptr()));
   ASSERT_FALSE(Int32Eq(kAValue, a_ptr));
   ASSERT_FALSE(IsMarked(a_ptr->raw_ptr()));
   ASSERT_TRUE(IsFree(a_ptr->raw_ptr()));

   //TODO: ASSERT_FALSE(IsInt32(a_ptr->raw_ptr()));
   ASSERT_FALSE(Int32Eq(kBValue, b_ptr));
   ASSERT_FALSE(IsMarked(b_ptr->raw_ptr()));
   ASSERT_TRUE(IsFree(b_ptr->raw_ptr()));
 }
}