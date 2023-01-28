#include <gtest/gtest.h>

#include "poseidon/flags.h"
#include "poseidon/type/all.h"
#include "marker/mock_marker.h"
#include "poseidon/local/local.h"
#include "helpers/alloc_helpers.h"
#include "assertions/assertions.h"
#include "matchers/is_pointer_to.h"
#include "poseidon/heap/page/new_page.h"
#include "poseidon/marker/marker_serial.h"

namespace poseidon {
 using namespace ::testing;

 class SerialMarkerTest : public Test {
  protected:
   MemoryRegion region_;

   SerialMarkerTest():
    Test(),
    region_(flags::GetOldZoneSize()) {
   }

   static inline bool
   SerialMark(Marker* marker) {
     SerialMarker serial_marker(marker);
     return serial_marker.MarkAllRoots();
   }
  public:
   ~SerialMarkerTest() override = default;

   void SetUp() override {
     ASSERT_TRUE(region_.Protect(MemoryRegion::kReadWrite));
     ASSERT_NO_FATAL_FAILURE(region_.ClearRegion());
   }

   void TearDown() override {
     ASSERT_TRUE(region_.Protect(MemoryRegion::kReadOnly));
   }
 };

 static inline bool
 MarkPointer(Pointer* raw_ptr) {
   DLOG(INFO) << "marking " << (*raw_ptr) << "....";
   raw_ptr->SetMarkedBit();
   return raw_ptr->IsMarked();
 }

 TEST_F(SerialMarkerTest, TestMarkAllRoots_WillFail_NoRootSet) {
   MockMarker marker;
   ASSERT_FALSE(SerialMark(&marker));
 }

 TEST_F(SerialMarkerTest, TestMarkAllRoots_WillPass_EmptyRootSet) {
   {
     LocalScope scope;

     MockMarker marker;
     ASSERT_TRUE(SerialMark(&marker));
   }
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

 TEST_F(SerialMarkerTest, TestMarkAllRoots_WillPass_MarksOne) {
   MemoryRegionScope mem_scope(flags::GetOldZoneSize(), MemoryRegion::kReadWrite);
   {
     LocalScope scope;
     const constexpr RawInt kAValue = 111; //TODO: clean up this allocation
     const Region r1 = Region::Subregion(mem_scope.region(), Int::GetClass()->GetAllocationSize() + static_cast<word>(sizeof(Pointer)));
     auto raw_a_ptr = Pointer::From(r1, PointerTag::Old(Int::GetClass()->GetAllocationSize()));
     const auto raw_a = Int::TryAllocateAt(raw_a_ptr->GetPointerRegion(), kAValue);
     DLOG(INFO) << "raw_a: " << *raw_a->raw_ptr();
     ASSERT_NE(raw_a, nullptr);
     ASSERT_TRUE(IsInt(raw_a->raw_ptr())) << (*raw_a->raw_ptr()) << " is not an Int";
     ASSERT_TRUE(IntEq(kAValue, raw_a));
     Local<Int> a;
     a = raw_a->raw_ptr();

     MockMarker marker;
     EXPECT_CALL(marker, Mark(IsPointerTo(raw_a_ptr)))
      .WillOnce(Return(true));
     ASSERT_TRUE(SerialMark(&marker));
   }
 }
}