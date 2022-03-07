#include <gtest/gtest.h>
#include <glog/logging.h>

#include "mock_zone.h"
#include "poseidon/scavenger.h"

namespace poseidon{
 class ScavengerTest : public ::testing::Test{
  public:
   ScavengerTest() = default;
   ~ScavengerTest() override = default;
 };

 TEST_F(ScavengerTest, TestScavenge){
   ::testing::StrictMock<MockZone> zone;

   Scavenger scavenger(&zone);
   scavenger.Scavenge();

   EXPECT_CALL(zone, SwapSpaces());
 }
}