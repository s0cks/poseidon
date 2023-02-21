#ifndef PSDN_TEST_MARKER_SERIAL_H
#define PSDN_TEST_MARKER_SERIAL_H

#include <gtest/gtest.h>
#include "poseidon/marker/marker_serial.h"

#include "base_memory_region_test.h"

namespace poseidon {
 class SerialMarkerTest : public BaseMemoryRegionTest {
  protected:
   NewZone zone_;

   SerialMarkerTest():
    BaseMemoryRegionTest(flags::GetNewZoneSize()),
    zone_(region_) {
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
}

#endif //PSDN_TEST_MARKER_SERIAL_H