#include <gtest/gtest.h>

#include "poseidon/type.h"

#include "assertions/assertions.h"
#include "matchers/is_pointer_to.h"
#include "mock_raw_object_visitor.h"

namespace poseidon {
 using namespace ::testing;

 class TupleTest : public Test {
  protected:
   TupleTest() = default;
  public:
   ~TupleTest() override = default;
 };

 TEST_F(TupleTest, TestNew_WillPass) {
   auto a = Tuple::New();
   DLOG(INFO) << "a: " << (*a);

   static const constexpr RawInt32 kBValue = 10;
   auto b = Int32::New(kBValue);
   DLOG(INFO) << "b: " << (*b) << " " << (*b->raw_ptr());
   ASSERT_NO_FATAL_FAILURE(a->SetCar(b->raw_ptr()));

   static const constexpr RawInt32 kCValue = 48;
   auto c = Int32::New(kCValue);
   DLOG(INFO) << "c: " << (*c) << " " << (*c->raw_ptr());
   ASSERT_NO_FATAL_FAILURE(a->SetCdr(c->raw_ptr()));

   DLOG(INFO) << "a: " << (*a);

   ASSERT_TRUE(IsInt32(a->GetCar<Int32>()->raw_ptr()));
   ASSERT_TRUE(Int32Eq(a->GetCar<Int32>(), b));
   ASSERT_TRUE(Int32Eq(a->GetCdr<Int32>(), c));
 }

 TEST_F(TupleTest, TestVisitPointers_WillPass) {
   auto a = Tuple::New();
   DLOG(INFO) << "a: " << (*a) << " " << (*a->raw_ptr());

   static const constexpr RawInt32 kBValue = 10;
   auto b = Int32::New(kBValue);
   DLOG(INFO) << "b: " << (*b) << " " << (*b->raw_ptr());
   ASSERT_NO_FATAL_FAILURE(a->SetCar(b->raw_ptr()));

   static const constexpr RawInt32 kCValue = 48;
   auto c = Int32::New(kCValue);
   DLOG(INFO) << "c: " << (*c) << " " << (*c->raw_ptr());
   ASSERT_NO_FATAL_FAILURE(a->SetCdr(c->raw_ptr()));

   DLOG(INFO) << "a: " << (*a);

   MockRawObjectVisitor visitor;
   EXPECT_CALL(visitor, Visit(IsPointerTo<Int32>(b)))
     .Times(1)
     .WillOnce(Return(true));
   EXPECT_CALL(visitor, Visit(IsPointerTo<Int32>(c)))
     .Times(1)
     .WillOnce(Return(true));
   ASSERT_EQ(a->raw_ptr()->VisitPointers(&visitor), a->raw_ptr()->GetSize());
 }
}