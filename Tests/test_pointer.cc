#include "poseidon/pointer.h"
#include "zone/base_new_zone_test.h"

namespace poseidon{
 using namespace ::testing;

 class PointerTest : public BaseNewZoneTest {
  protected:
   PointerTest() = default;
  public:
   ~PointerTest() override = default;

   void SetUp() override {
     BaseNewZoneTest::SetUp();
   }

   void TearDown() override {
     BaseNewZoneTest::TearDown();
   }
 };
}