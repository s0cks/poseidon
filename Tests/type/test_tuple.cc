#include <gtest/gtest.h>

#include "poseidon/type/int.h"
#include "poseidon/type/tuple.h"

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

   static const constexpr RawInt kBValue = 10;
   auto b = Int::New(kBValue);
   DLOG(INFO) << "b: " << (*b) << " " << (*b->raw_ptr());
   ASSERT_NO_FATAL_FAILURE(a->SetCar(b->raw_ptr()));

   static const constexpr RawInt kCValue = 48;
   auto c = Int::New(kCValue);
   DLOG(INFO) << "c: " << (*c) << " " << (*c->raw_ptr());
   ASSERT_NO_FATAL_FAILURE(a->SetCdr(c->raw_ptr()));

   DLOG(INFO) << "a: " << (*a);

   ASSERT_TRUE(IsInt(a->GetCar<Int>()->raw_ptr()));
   ASSERT_TRUE(IntEq(a->GetCar<Int>(), b));
   ASSERT_TRUE(IntEq(a->GetCdr<Int>(), c));
 }

 TEST_F(TupleTest, TestVisitPointers_WillPass) {
   auto a = Tuple::New();
   DLOG(INFO) << "a: " << (*a) << " " << (*a->raw_ptr());

   static const constexpr RawInt kBValue = 10;
   auto b = Int::New(kBValue);
   DLOG(INFO) << "b: " << (*b) << " " << (*b->raw_ptr());
   ASSERT_NO_FATAL_FAILURE(a->SetCar(b->raw_ptr()));

   static const constexpr RawInt kCValue = 48;
   auto c = Int::New(kCValue);
   DLOG(INFO) << "c: " << (*c) << " " << (*c->raw_ptr());
   ASSERT_NO_FATAL_FAILURE(a->SetCdr(c->raw_ptr()));

   DLOG(INFO) << "a: " << (*a);

   MockRawObjectVisitor visitor;
   EXPECT_CALL(visitor, Visit(IsPointerTo<Int>(b)))
     .Times(1)
     .WillOnce(Return(true));
   EXPECT_CALL(visitor, Visit(IsPointerTo<Int>(c)))
     .Times(1)
     .WillOnce(Return(true));
   ASSERT_EQ(a->raw_ptr()->VisitPointers(&visitor), a->raw_ptr()->GetSize());
 }
}