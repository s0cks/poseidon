#include <gtest/gtest.h>

#include "helpers.h"
#include "poseidon/flags.h"
#include "marker/mock_marker.h"
#include "helpers/alloc_helpers.h"
#include "poseidon/heap/new_page.h"
#include "poseidon/marker/marker_serial.h"

namespace poseidon {
 using namespace ::testing;

 class SerialMarkerTest : public Test {
  protected:
   SerialMarkerTest() = default;

   static inline bool
   SerialMark(Marker* marker) {
     SerialMarker serial_marker(marker);
     return serial_marker.MarkAllRoots();
   }
  public:
   ~SerialMarkerTest() override = default;
 };

 TEST_F(SerialMarkerTest, TestMarkAllRoots_WillPass) {
   ASSERT_NO_FATAL_FAILURE(LocalPage::Initialize());

   static const int64_t kNewZoneTotalSize = GetNewZoneSize();
   MemoryRegion new_zone_region(kNewZoneTotalSize);
   ASSERT_TRUE(new_zone_region.Protect(MemoryRegion::kReadWrite));
   const auto new_zone = NewZone::New(new_zone_region);

   static const int64_t kOldZoneTotalSize = GetOldZoneSize();
   MemoryRegion old_zone_region(kOldZoneTotalSize);
   ASSERT_TRUE(old_zone_region.Protect(MemoryRegion::kReadWrite));
   const auto old_zone = OldZone::From(old_zone_region);

   static constexpr const word kAValue = 1394;
   Local<word> a(TryAllocateWord(new_zone, kAValue));
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsNewWord(a, kAValue));
   ASSERT_FALSE(IsMarked(a));

   static constexpr const word kBValue = 595;
   Local<word> b(TryAllocateWord(old_zone, kBValue));
   ASSERT_TRUE(IsAllocated(b));
   ASSERT_TRUE(IsOldWord(b, kBValue));
   ASSERT_FALSE(IsMarked(b));

   static constexpr const word kCValue = 3848;
   Local<word> c(TryAllocateWord(new_zone, kCValue));
   ASSERT_TRUE(IsAllocated(c));
   ASSERT_TRUE(IsNewWord(c, kCValue));
   ASSERT_FALSE(IsMarked(c));

   MockMarker marker;
   EXPECT_CALL(marker, Mark(IsPointerTo(a.raw())))
    .Times(1)
    .WillOnce([](RawObject* ptr) {
      DLOG(INFO) << "marking " << (*ptr);
      ptr->SetMarkedBit();
      return ptr->IsMarked();
    });
   EXPECT_CALL(marker, Mark(IsPointerTo(b.raw())))
    .Times(1)
    .WillOnce([](RawObject* ptr) {
      DLOG(INFO) << "marking " << (*ptr);
      ptr->SetMarkedBit();
      return ptr->IsMarked();
    });
   EXPECT_CALL(marker, Mark(IsPointerTo(c.raw())))
    .Times(1)
    .WillOnce([](RawObject* ptr) {
      DLOG(INFO) << "marking " << (*ptr);
      ptr->SetMarkedBit();
      return ptr->IsMarked();
    });

   ASSERT_NO_FATAL_FAILURE(SerialMark(&marker));

   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsNewWord(a, kAValue));
   ASSERT_TRUE(IsMarked(a));

   ASSERT_TRUE(IsAllocated(b));
   ASSERT_TRUE(IsOldWord(b, kBValue));
   ASSERT_TRUE(IsMarked(b));

   ASSERT_TRUE(IsAllocated(c));
   ASSERT_TRUE(IsNewWord(c, kCValue));
   ASSERT_TRUE(IsMarked(c));
 }

 TEST_F(SerialMarkerTest, TestMarkAllNewRoots_WillPass) {
   NOT_IMPLEMENTED(ERROR); //TODO: implement
 }

 TEST_F(SerialMarkerTest, TestMarkAllOldRoots_WillPass) {
   NOT_IMPLEMENTED(ERROR); //TODO: implement
 }
}