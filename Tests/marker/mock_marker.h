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
    Marker() {
     ON_CALL(*this, Mark)
      .WillByDefault(Return(false));
   }
   ~MockMarker() override = default;
   MOCK_METHOD(bool, Mark, (RawObject*), (override));
 };
}

#endif // POSEIDON_MOCK_MARKER_H