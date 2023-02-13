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
     ON_CALL(*this, Mark(_))
      .WillByDefault([&](Pointer* ptr) {
        return Marker::Mark(ptr);
      });
   }
   ~MockMarker() override = default;
   MOCK_METHOD(void, Mark, (Pointer*), (override));
 };
}

#endif // POSEIDON_MOCK_MARKER_H