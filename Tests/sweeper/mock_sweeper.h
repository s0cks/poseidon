#ifndef POSEIDON_MOCK_SWEEPER_H
#define POSEIDON_MOCK_SWEEPER_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "poseidon/sweeper/sweeper.h"

namespace poseidon {
 class MockSweeper : public Sweeper {
  public:
   explicit MockSweeper(OldZone* zone):
    Sweeper(zone) {
   }
   ~MockSweeper() override = default;
   MOCK_METHOD(bool, Sweep, (Pointer*), (override));
 };
}

#endif // POSEIDON_MOCK_SWEEPER_H