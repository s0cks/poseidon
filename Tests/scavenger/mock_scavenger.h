#ifndef POSEIDON_MOCK_SCAVENGER_H
#define POSEIDON_MOCK_SCAVENGER_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "poseidon/scavenger/scavenger.h"

namespace poseidon {
 using namespace ::testing;

 class MockScavenger : public Scavenger {
  public:
   explicit MockScavenger(Heap* heap):
    Scavenger(heap) {
     ON_CALL(*this, Scavenge(_))
      .WillByDefault([this](Pointer* raw_ptr) {
        return Scavenger::Scavenge(raw_ptr);
      });
     ON_CALL(*this, Promote(_))
      .WillByDefault([this](Pointer* raw_ptr) {
        return Scavenger::Promote(raw_ptr);
      });
   }
   ~MockScavenger() override = default;
   MOCK_METHOD(uword, Promote, (Pointer*), (override));
   MOCK_METHOD(uword, Scavenge, (Pointer*), (override));
 };
}

#endif // POSEIDON_MOCK_SCAVENGER_H