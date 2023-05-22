#ifndef PSDN_TEST_SCAVENGER_SERIAL_H
#define PSDN_TEST_SCAVENGER_SERIAL_H

#include <gtest/gtest.h>
#include "mock_scavenger.h"
#include "zone/mock_new_zone.h"
#include "poseidon/scavenger/scavenger_serial.h"

namespace poseidon {
 class SerialScavengerTest : public ::testing::Test {
  protected:
   MockNewZone zone_;

   SerialScavengerTest() = default;

   inline NewZone& zone() {
     return zone_;
   }

   inline void SerialScavenge(MockScavenger* scavenger) {
     SerialScavenger serial_scavenger(scavenger, &zone(), nullptr);
     return serial_scavenger.ScavengeMemory();
   }
  public:
   ~SerialScavengerTest() override = default;
 };
}

#endif //PSDN_TEST_SCAVENGER_SERIAL_H