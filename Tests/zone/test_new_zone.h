#ifndef PSDN_TEST_NEW_ZONE_H
#define PSDN_TEST_NEW_ZONE_H

#include <gtest/gtest.h>
#include "poseidon/marker/marker.h"
#include "poseidon/zone/new_zone.h"

namespace poseidon {
 class NewZoneTest : public ::testing::Test {
  protected:
   MemoryRegion region_;
   NewZone zone_;
   Marker marker_;

   NewZoneTest():
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

   inline Marker& marker() {
     return marker_;
   }

   inline bool Mark(Pointer* ptr) {
     return marker().Visit(ptr);
   }

   template<class T>
   inline bool Mark(T* value) {
     return Mark(value->raw_ptr());
   }
  public:
   ~NewZoneTest() override = default;

   void SetUp() override {
     ASSERT_NO_FATAL_FAILURE(zone().SetWritable());
     ASSERT_NO_FATAL_FAILURE(zone().Clear());
     ASSERT_NO_FATAL_FAILURE(zone().SetReadOnly());
     ASSERT_NO_FATAL_FAILURE(NewZonePrinter::Print(&zone()));
     ASSERT_NO_FATAL_FAILURE(zone().SetWritable());
   }

   void TearDown() override {
     ASSERT_NO_FATAL_FAILURE(zone().SetReadOnly());
     ASSERT_NO_FATAL_FAILURE(NewZonePrinter::Print(&zone()));
   }
 };
}

#endif //PSDN_TEST_NEW_ZONE_H