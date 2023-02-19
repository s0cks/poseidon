#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "poseidon/object.h"
#include "poseidon/local/local.h"
#include "poseidon/zone/new_zone.h"
#include "poseidon/marker/marker_parallel.h"

#include "marker/mock_marker.h"
#include "matchers/is_pointer_to.h"
#include "assertions/ptr_assertions.h"
#include "assertions/type_assertions.h"

namespace poseidon {
 using namespace ::testing;

 class ParallelMarkerTest : public Test {
  protected:
   MemoryRegion region_;
   NewZone zone_;

   ParallelMarkerTest():
    Test(),
    region_(flags::GetNewZoneSize(), MemoryRegion::kReadWrite),
    zone_(region_) {
   }

   inline MemoryRegion& region() {
     return region_;
   }

   inline NewZone& zone() {
     return zone_;
   }

   static inline void
   ParallelMark(Marker* marker) {
     auto m = new ParallelMarker(marker);
     m->MarkAllRoots();
     while(Marker::IsMarking()); //spin
   }
  public:
   ~ParallelMarkerTest() override = default;

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

 TEST_F(ParallelMarkerTest, TestMarkAllRoots_WillFail_NoRootSet) {
   MockMarker marker;
   EXPECT_DEATH(ParallelMark(&marker), "no local page exists for current thread.");
 }

 TEST_F(ParallelMarkerTest, TestMarkAllRoots_WillPass_EmptyRootSet) {
   LocalScope scope;

   MockMarker marker;
   ASSERT_NO_FATAL_FAILURE(ParallelMark(&marker));
 }

 TEST_F(ParallelMarkerTest, TestMarkAllRoots_WillPass_MarksOneInt32) {
   LocalScope scope;
   const constexpr RawInt32 kAValue = 111;
   Local<Int32> a(Int32::TryAllocateIn(&zone(), kAValue));
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsInt32(a));
   ASSERT_TRUE(Int32Eq(kAValue, a));
   ASSERT_FALSE(IsMarked(a));

   MockMarker marker;
   ASSERT_NO_FATAL_FAILURE(ParallelMark(&marker));

   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsInt32(a));
   ASSERT_TRUE(Int32Eq(kAValue, a));
   ASSERT_TRUE(IsMarked(a));
 }

 TEST_F(ParallelMarkerTest, TestMarkAllRoots_WillPass_MarksMultipleInt32s) {
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
   ASSERT_NO_FATAL_FAILURE(ParallelMark(&marker));

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

 TEST_F(ParallelMarkerTest, TestMarkAllRoots_WillPass_MarksTupleCarAndCdr) {
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
   ASSERT_NO_FATAL_FAILURE(ParallelMark(&marker));

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

 TEST_F(ParallelMarkerTest, TestMarkAllRoots_WillPass_MarksMultipleTuples) {
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

   Local<Tuple> f(Tuple::TryAllocateIn(&zone()));
   ASSERT_TRUE(IsAllocated(f));
   ASSERT_TRUE(IsTuple(f));
   ASSERT_TRUE(IsUnallocated(f->GetCarPointer()));
   ASSERT_NO_FATAL_FAILURE(f->SetCar(b));
   ASSERT_TRUE(IsAllocated(f->GetCarPointer()));
   ASSERT_TRUE(IsInt32(f->GetCarPointer()));
   ASSERT_TRUE(Int32Eq(f->GetCar<Int32>(), b));
   ASSERT_TRUE(IsUnallocated(f->GetCdrPointer()));
   ASSERT_NO_FATAL_FAILURE(f->SetCdr(d));
   ASSERT_TRUE(IsAllocated(f->GetCdrPointer()));
   ASSERT_TRUE(IsInt32(f->GetCdrPointer()));
   ASSERT_TRUE(Int32Eq(f->GetCdr<Int32>(), d));
   DLOG(INFO) << "f: " << (*f.raw_ptr()); //TODO: print tuple

   MockMarker marker;
   ASSERT_NO_FATAL_FAILURE(ParallelMark(&marker));

   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsInt32(a));
   ASSERT_TRUE(Int32Eq(kAValue, a));
   ASSERT_TRUE(IsMarked(a));

   ASSERT_TRUE(IsAllocated(b));
   ASSERT_TRUE(IsInt32(b));
   ASSERT_TRUE(Int32Eq(kBValue, b));
   ASSERT_TRUE(IsMarked(b));

   ASSERT_TRUE(IsAllocated(c));
   ASSERT_TRUE(IsInt32(c));
   ASSERT_TRUE(Int32Eq(kCValue, c));
   ASSERT_TRUE(IsMarked(c));

   ASSERT_TRUE(IsAllocated(d));
   ASSERT_TRUE(IsInt32(d));
   ASSERT_TRUE(Int32Eq(kDValue, d));
   ASSERT_TRUE(IsMarked(d));

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

   ASSERT_TRUE(IsAllocated(f));
   ASSERT_TRUE(IsTuple(f));
   ASSERT_TRUE(IsMarked(f));
   ASSERT_TRUE(IsAllocated(f->GetCarPointer()));
   ASSERT_TRUE(IsInt32(f->GetCarPointer()));
   ASSERT_TRUE(Int32Eq(f->GetCar<Int32>(), b));
   ASSERT_TRUE(IsMarked(f->GetCarPointer()));
   ASSERT_TRUE(IsAllocated(f->GetCdrPointer()));
   ASSERT_TRUE(IsInt32(f->GetCdrPointer()));
   ASSERT_TRUE(Int32Eq(f->GetCdr<Int32>(), d));
   ASSERT_TRUE(IsMarked(f->GetCdrPointer()));
 }
}