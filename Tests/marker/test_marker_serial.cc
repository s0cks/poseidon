#include "marker/test_marker_serial.h"

#include "poseidon/object.h"
#include "poseidon/flags.h"
#include "marker/mock_marker.h"
#include "poseidon/local/local.h"
#include "matchers/is_pointer_to.h"
#include "assertions/assertions.h"

namespace poseidon {
 using namespace ::testing;

 TEST_F(SerialMarkerTest, TestMarkAllRoots_WillFail_NoRootSet) {
   MockMarker marker;
   EXPECT_DEATH(SerialMark(&marker), "no local page exists for current thread.");
 }

 TEST_F(SerialMarkerTest, TestMarkAllRoots_WillPass_EmptyRootSet) {
   LocalScope scope;
   MockMarker marker;
   ASSERT_NO_FATAL_FAILURE(SerialMark(&marker));
 }

 TEST_F(SerialMarkerTest, TestMarkAllRoots_WillPass_MarksOneNull) {
   LocalScope scope;
   Local<Null> a(Null::TryAllocateIn(&zone()));
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsNull(a));
   ASSERT_FALSE(IsMarked(a));
   MockMarker marker;
   EXPECT_CALL(marker, Mark(IsPointerTo(a)));
   ASSERT_NO_FATAL_FAILURE(SerialMark(&marker));
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsNull(a));
   ASSERT_TRUE(IsMarked(a));
 }

 TEST_F(SerialMarkerTest, TestMarkAllRoots_WillPass_MarksOneTrue) {
   LocalScope scope;
   Local<Bool> a(Bool::TryAllocateIn(&zone(), true));
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsBool(a));
   ASSERT_TRUE(BoolEq(true, a));
   ASSERT_FALSE(IsMarked(a));
   MockMarker marker;
   EXPECT_CALL(marker, Mark(IsPointerTo(a)));
   ASSERT_NO_FATAL_FAILURE(SerialMark(&marker));
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsBool(a));
   ASSERT_TRUE(BoolEq(true, a));
   ASSERT_TRUE(IsMarked(a));
 }

 TEST_F(SerialMarkerTest, TestMarkAllRoots_WillPass_MarksOneFalse) {
   LocalScope scope;
   Local<Bool> a(Bool::TryAllocateIn(&zone(), false));
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsBool(a));
   ASSERT_TRUE(BoolEq(false, a));
   ASSERT_FALSE(IsMarked(a));
   MockMarker marker;
   EXPECT_CALL(marker, Mark(IsPointerTo(a)));
   ASSERT_NO_FATAL_FAILURE(SerialMark(&marker));
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsBool(a));
   ASSERT_TRUE(BoolEq(false, a));
   ASSERT_TRUE(IsMarked(a));
 }

#define DEFINE_SERIAL_MARK_NUMBER_TYPE_PASSES_SERIAL_MARKER_TEST(Type) \
 TEST_F(SerialMarkerTest, TestMarkAllRoots_WillPass_MarksOne##Type) {  \
   LocalScope scope;                                                   \
   const constexpr Raw##Type kAValue = 111;                            \
   Local<Type> a(Type::TryAllocateIn(&zone(), kAValue));               \
   ASSERT_TRUE(IsAllocated(a));                                        \
   ASSERT_TRUE(Is##Type(a));                                           \
   ASSERT_TRUE(Type##Eq(kAValue, a));                                  \
   ASSERT_FALSE(IsMarked(a));                                          \
   MockMarker marker;                                                  \
   EXPECT_CALL(marker, Mark(IsPointerTo(a)));                          \
   ASSERT_NO_FATAL_FAILURE(SerialMark(&marker));                       \
   ASSERT_TRUE(IsAllocated(a));                                        \
   ASSERT_TRUE(Is##Type(a));                                           \
   ASSERT_TRUE(Type##Eq(kAValue, a));                                  \
   ASSERT_TRUE(IsMarked(a));                                           \
 }
 FOR_EACH_INT_TYPE(DEFINE_SERIAL_MARK_NUMBER_TYPE_PASSES_SERIAL_MARKER_TEST);
#undef DEFINE_SERIAL_MARK_NUMBER_TYPE_PASSES_SERIAL_MARKER_TEST

 TEST_F(SerialMarkerTest, TestMarkAllRoots_WillPass_MarksOneTuple) {
   LocalScope scope;
   MockMarker marker;

   Local<Tuple> a(Tuple::TryAllocateIn(&zone()));
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsTuple(a));
   ASSERT_FALSE(IsMarked(a));
   EXPECT_CALL(marker, Mark(IsPointerTo(a)));

   ASSERT_NO_FATAL_FAILURE(SerialMark(&marker));

   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsTuple(a));
   ASSERT_TRUE(IsMarked(a));
 }

 TEST_F(SerialMarkerTest, TestMarkAllRoots_WillPass_MarksTupleCarAndCdr) {
   LocalScope scope;
   MockMarker marker;

   const constexpr RawInt32 kAValue = 333;
   auto a = Int32::TryAllocateIn(&zone(), kAValue);
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsInt32(a));
   ASSERT_TRUE(Int32Eq(kAValue, a));
   ASSERT_FALSE(IsMarked(a));
   EXPECT_CALL(marker, Mark(IsPointerTo(a)));

   const constexpr RawInt32 kBValue = 444;
   auto b = Int32::TryAllocateIn(&zone(), kBValue);
   ASSERT_TRUE(IsAllocated(b));
   ASSERT_TRUE(IsInt32(b));
   ASSERT_TRUE(Int32Eq(kBValue, b));
   ASSERT_FALSE(IsMarked(b));

   const constexpr RawInt32 kCValue = 444;
   auto c = Int32::TryAllocateIn(&zone(), kCValue);
   ASSERT_TRUE(IsAllocated(c));
   ASSERT_TRUE(IsInt32(c));
   ASSERT_TRUE(Int32Eq(kCValue, c));
   ASSERT_FALSE(IsMarked(c));
   EXPECT_CALL(marker, Mark(IsPointerTo(c)));

   const constexpr RawInt32 kDValue = 444;
   auto d = Int32::TryAllocateIn(&zone(), kDValue);
   ASSERT_TRUE(IsAllocated(d));
   ASSERT_TRUE(IsInt32(d));
   ASSERT_TRUE(Int32Eq(kDValue, d));
   ASSERT_FALSE(IsMarked(d));

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
   EXPECT_CALL(marker, Mark(IsPointerTo(e)));

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
   EXPECT_CALL(marker, Mark(IsPointerTo(a.raw_ptr())));
   EXPECT_CALL(marker, Mark(IsPointerTo(c.raw_ptr())));
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
}