#include <gtest/gtest.h>

#include "helpers.h"
#include "poseidon/flags.h"
#include "poseidon/type/long.h"
#include "marker/mock_marker.h"
#include "helpers/alloc_helpers.h"
#include "poseidon/local/local.h"
#include "poseidon/heap/new_page.h"
#include "poseidon/marker/marker_serial.h"

#include "matchers/is_pointer_to.h"
#include "assertions/type_assertions.h"

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

 static inline bool
 MarkPointer(Pointer* raw_ptr) {
   DLOG(INFO) << "marking " << (*raw_ptr) << "....";
   raw_ptr->SetMarkedBit();
   return raw_ptr->IsMarked();
 }

 TEST_F(SerialMarkerTest, TestMarkAllRoots_WillPass) {
   MemoryRegion page_region(LocalPage::CalculateLocalPageSize(64));
   ASSERT_TRUE(page_region.Protect(MemoryRegion::kReadWrite));
   ASSERT_NO_FATAL_FAILURE(LocalPage::SetForCurrentThread(new LocalPage(page_region)));

   NewZone new_zone(GetNewZoneSize(), GetNewZoneSemispaceSize());
   ASSERT_NO_FATAL_FAILURE(new_zone.SetWritable());

   OldZone old_zone(GetOldZoneSize());
   ASSERT_NO_FATAL_FAILURE(old_zone.SetWritable());

   MockMarker marker;

   static constexpr const RawLong kAValue = 1394;
   auto raw_a = Long::TryAllocateIn(&new_zone, kAValue);
   ASSERT_NE(raw_a, nullptr);
   ASSERT_TRUE(IsLong(raw_a->raw_ptr()));
   ASSERT_TRUE(LongEq(kAValue, raw_a));
   Local<Long> a(raw_a->raw_ptr());
   ASSERT_FALSE(IsMarked(a));
   ON_CALL(marker, Mark(IsPointerTo(raw_a->raw_ptr())))
    .WillByDefault(MarkPointer);
   EXPECT_CALL(marker, Mark(IsPointerTo(raw_a->raw_ptr())))
    .WillOnce(Return(true));

   static constexpr const RawLong kBValue = 595;
   auto raw_b = Long::TryAllocateIn(&old_zone, kBValue);
   ASSERT_NE(raw_b, nullptr);
   ASSERT_TRUE(IsLong(raw_b->raw_ptr()));
   ASSERT_TRUE(LongEq(kBValue, raw_b));
   Local<Long> b(raw_b->raw_ptr());
   ASSERT_FALSE(IsMarked(b));
   ON_CALL(marker, Mark(IsPointerTo(raw_b->raw_ptr())))
    .WillByDefault(MarkPointer);
   EXPECT_CALL(marker, Mark(IsPointerTo(raw_b->raw_ptr())))
    .WillOnce(Return(true));

   static constexpr const RawLong kCValue = 3848;
   auto raw_c = Long::TryAllocateIn(&old_zone, kCValue);
   ASSERT_NE(raw_c, nullptr);
   ASSERT_TRUE(IsLong(raw_c->raw_ptr()));
   ASSERT_TRUE(LongEq(kCValue, raw_c));
   Local<Long> c(raw_c->raw_ptr());
   ON_CALL(marker, Mark(IsPointerTo(raw_c->raw_ptr())))
    .WillByDefault(MarkPointer);
   EXPECT_CALL(marker, Mark(IsPointerTo(raw_c->raw_ptr())))
    .WillOnce(Return(true));

   ASSERT_NO_FATAL_FAILURE(SerialMark(&marker));

   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsNewLong(a, kAValue));
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