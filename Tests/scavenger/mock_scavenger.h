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
      .WillByDefault([](RawObject* ptr) {
        return UNALLOCATED;
      });
     ON_CALL(*this, Promote(_))
      .WillByDefault([](RawObject* ptr) {
        return UNALLOCATED;
      });
   }
   ~MockScavenger() override = default;
   MOCK_METHOD(uword, Promote, (RawObject*), (override));
   MOCK_METHOD(uword, Scavenge, (RawObject*), (override));
 };
}

#endif // POSEIDON_MOCK_SCAVENGER_H