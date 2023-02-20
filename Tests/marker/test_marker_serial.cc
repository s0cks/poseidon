#include <gtest/gtest.h>

#include "poseidon/object.h"
#include "poseidon/flags.h"
#include "marker/mock_marker.h"
#include "poseidon/local/local.h"
#include "helpers/alloc_helpers.h"
#include "matchers/is_pointer_to.h"
#include "assertions/assertions.h"
#include "poseidon/page/new_page.h"
#include "poseidon/marker/marker_serial.h"

namespace poseidon {
 using namespace ::testing;

 class SerialMarkerTest : public Test {
  protected:
   MemoryRegion region_;
   NewZone zone_;

   SerialMarkerTest():
    Test(),
    region_(flags::GetNewZoneSize()),
    zone_(region_) {
   }

   inline MemoryRegion& region() {
     return region_;
   }

   inline NewZone& zone() {
     return zone_;
   }

   static inline void
   SerialMark(Marker* marker) {
     SerialMarker serial_marker(marker);
     return serial_marker.MarkAllRoots();
   }
  public:
   ~SerialMarkerTest() override = default;

   void SetUp() override {
#ifdef PSDN_DEBUG
     ASSERT_NO_FATAL_FAILURE(zone().SetReadOnly());
     NewZonePrinter::Print(&zone());
#endif //PSDN_DEBUG
     ASSERT_NO_FATAL_FAILURE(zone().SetWritable());
     ASSERT_NO_FATAL_FAILURE(zone().Clear());
   }

   void TearDown() override {
     ASSERT_NO_FATAL_FAILURE(zone().SetReadOnly());
#ifdef PSDN_DEBUG
     NewZonePrinter::Print(&zone());
#endif //PSDN_DEBUG
   }
 };

 TEST_F(SerialMarkerTest, TestMarkAllRoots_WillFail_NoRootSet) {
   MockMarker marker;
   EXPECT_DEATH(SerialMark(&marker), "no local page exists for current thread.");
 }

 TEST_F(SerialMarkerTest, TestMarkAllRoots_WillPass_EmptyRootSet) {
   LocalScope scope;

   MockMarker marker;
   ASSERT_NO_FATAL_FAILURE(SerialMark(&marker));
 }

 class MemoryRegionScope {
  protected:
   MemoryRegion region_;
  public:
   explicit MemoryRegionScope(const RegionSize size,
                              MemoryRegion::ProtectionMode mode = MemoryRegion::kNoAccess):
    region_(size) {
     LOG_IF(FATAL, !region_.Protect(mode)) << "failed to protect " << region_;
   }
   ~MemoryRegionScope() {
     region_.FreeRegion();
   }

   const MemoryRegion& region() const {
     return region_;
   }

   MemoryRegion* operator->() {
     return &region_;
   }

   explicit operator MemoryRegion*() {
     return &region_;
   }

   explicit operator const MemoryRegion&() const {
     return region_;
   }
 };

 TEST_F(SerialMarkerTest, TestMarkAllRoots_WillPass_MarksOneInt32) {
   LocalScope scope;
   const constexpr RawInt32 kAValue = 111;
   Local<Int32> a(Int32::TryAllocateIn(&zone(), kAValue));
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsInt32(a));
   ASSERT_TRUE(Int32Eq(kAValue, a));
   ASSERT_FALSE(IsMarked(a));

   MockMarker marker;
   EXPECT_CALL(marker, Mark(IsPointerTo(a.raw_ptr())))
    .Times(1);
   ASSERT_NO_FATAL_FAILURE(SerialMark(&marker));

   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsInt32(a));
   ASSERT_TRUE(Int32Eq(kAValue, a));
   ASSERT_TRUE(IsMarked(a));
 }

 TEST_F(SerialMarkerTest, TestMarkAllRoots_WillPass_MarksMultipleInt32s) {
   LocalScope scope;
   const constexpr RawInt32 kAValue = 111;
   Local<Int32> a(Int32::TryAllocateIn(&zone(), kAValue));
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsInt32(a));
   ASSERT_TRUE(Int32Eq(kAValue, a));
   ASSERT_FALSE(IsMarked(a));

   const constexpr RawInt32 kBValue = 333;
   auto b = Int32::TryAllocateIn(&zone(), kBValue);
   ASSERT_TRUE(IsAllocated(b));
   ASSERT_TRUE(IsInt32(b));
   ASSERT_TRUE(Int32Eq(kBValue, b));
   ASSERT_FALSE(IsMarked(b));

   const constexpr RawInt32 kCValue = 222;
   Local<Int32> c(Int32::TryAllocateIn(&zone(), kCValue));
   ASSERT_TRUE(IsAllocated(c));
   ASSERT_TRUE(IsInt32(c));
   ASSERT_TRUE(Int32Eq(kCValue, c));
   ASSERT_FALSE(IsMarked(c));

   MockMarker marker;
   EXPECT_CALL(marker, Mark(IsPointerTo(a.raw_ptr())))
    .Times(1);
   EXPECT_CALL(marker, Mark(IsPointerTo(c.raw_ptr())))
    .Times(1);
   ASSERT_NO_FATAL_FAILURE(SerialMark(&marker));

   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsInt32(a));
   ASSERT_TRUE(Int32Eq(kAValue, a));
   ASSERT_TRUE(IsMarked(a));

   ASSERT_TRUE(IsAllocated(b));
   ASSERT_TRUE(IsInt32(b));
   ASSERT_TRUE(Int32Eq(kBValue, b));
   ASSERT_FALSE(IsMarked(b));

   ASSERT_TRUE(IsAllocated(c));
   ASSERT_TRUE(IsInt32(c));
   ASSERT_TRUE(Int32Eq(kCValue, c));
   ASSERT_TRUE(IsMarked(c));
 }

 TEST_F(SerialMarkerTest, TestMarkAllRoots_WillPass_MarksTupleCarAndCdr) {
   LocalScope scope;
   const constexpr RawInt32 kAValue = 333;
   auto a = Int32::TryAllocateIn(&zone(), kAValue);
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsInt32(a));
   ASSERT_TRUE(Int32Eq(kAValue, a));
   ASSERT_FALSE(IsMarked(a));
   DLOG(INFO) << "a: " << (*a);

   const constexpr RawInt32 kBValue = 444;
   auto b = Int32::TryAllocateIn(&zone(), kBValue);
   ASSERT_TRUE(IsAllocated(b));
   ASSERT_TRUE(IsInt32(b));
   ASSERT_TRUE(Int32Eq(kBValue, b));
   ASSERT_FALSE(IsMarked(b));
   DLOG(INFO) << "b: " << (*b);

   const constexpr RawInt32 kCValue = 444;
   auto c = Int32::TryAllocateIn(&zone(), kCValue);
   ASSERT_TRUE(IsAllocated(c));
   ASSERT_TRUE(IsInt32(c));
   ASSERT_TRUE(Int32Eq(kCValue, c));
   ASSERT_FALSE(IsMarked(c));
   DLOG(INFO) << "c: " << (*c);

   const constexpr RawInt32 kDValue = 444;
   auto d = Int32::TryAllocateIn(&zone(), kDValue);
   ASSERT_TRUE(IsAllocated(d));
   ASSERT_TRUE(IsInt32(d));
   ASSERT_TRUE(Int32Eq(kDValue, d));
   ASSERT_FALSE(IsMarked(d));
   DLOG(INFO) << "d: " << (*d);

   Local<Tuple> e(Tuple::TryAllocateIn(&zone()));
   ASSERT_TRUE(IsAllocated(e));
   ASSERT_TRUE(IsTuple(e));
   ASSERT_TRUE(IsUnallocated(e->GetCarPointer()));
   ASSERT_NO_FATAL_FAILURE(e->SetCar(a));
   ASSERT_TRUE(IsAllocated(e->GetCarPointer()));
   ASSERT_TRUE(IsInt32(e->GetCarPointer()));
   ASSERT_TRUE(Int32Eq(e->GetCar<Int32>(), a));
   ASSERT_TRUE(IsUnallocated(e->GetCdrPointer()));
   ASSERT_NO_FATAL_FAILURE(e->SetCdr(c));
   ASSERT_TRUE(IsAllocated(e->GetCdrPointer()));
   ASSERT_TRUE(IsInt32(e->GetCdrPointer()));
   ASSERT_TRUE(Int32Eq(e->GetCdr<Int32>(), c));
   DLOG(INFO) << "e: " << (*e.raw_ptr()); //TODO: print tuple

   MockMarker marker;
   EXPECT_CALL(marker, Mark(IsPointerTo(e)))
    .Times(1);
   EXPECT_CALL(marker, Mark(IsPointerTo(a)))
    .Times(1);
   EXPECT_CALL(marker, Mark(IsPointerTo(c)))
    .Times(1);
   ASSERT_NO_FATAL_FAILURE(SerialMark(&marker));

   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsInt32(a));
   ASSERT_TRUE(Int32Eq(kAValue, a));
   ASSERT_TRUE(IsMarked(a));

   ASSERT_TRUE(IsAllocated(b));
   ASSERT_TRUE(IsInt32(b));
   ASSERT_TRUE(Int32Eq(kBValue, b));
   ASSERT_FALSE(IsMarked(b));

   ASSERT_TRUE(IsAllocated(c));
   ASSERT_TRUE(IsInt32(c));
   ASSERT_TRUE(Int32Eq(kCValue, c));
   ASSERT_TRUE(IsMarked(c));

   ASSERT_TRUE(IsAllocated(d));
   ASSERT_TRUE(IsInt32(d));
   ASSERT_TRUE(Int32Eq(kDValue, d));
   ASSERT_FALSE(IsMarked(d));

   ASSERT_TRUE(IsAllocated(e));
   ASSERT_TRUE(IsTuple(e));
   ASSERT_TRUE(IsMarked(e));
   ASSERT_TRUE(IsAllocated(e->GetCarPointer()));
   ASSERT_TRUE(IsInt32(e->GetCarPointer()));
   ASSERT_TRUE(Int32Eq(e->GetCar<Int32>(), a));
   ASSERT_TRUE(IsMarked(e->GetCarPointer()));
   ASSERT_TRUE(IsAllocated(e->GetCdrPointer()));
   ASSERT_TRUE(IsInt32(e->GetCdrPointer()));
   ASSERT_TRUE(Int32Eq(e->GetCdr<Int32>(), c));
   ASSERT_TRUE(IsMarked(e->GetCdrPointer()));
 }
}