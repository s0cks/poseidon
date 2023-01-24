#ifndef POSEIDON_MOCK_MARKER_H
#define POSEIDON_MOCK_MARKER_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "poseidon/marker/marker.h"

namespace poseidon {
 using namespace ::testing;

 class MockMarker : public Marker {
  public:
   explicit MockMarker():
    Marker() { }
   ~MockMarker() override = default;
   MOCK_METHOD(bool, Mark, (Pointer*), (override));
 };
}

#endif // POSEIDON_MOCK_MARKER_H