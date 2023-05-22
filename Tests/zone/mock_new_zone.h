#ifndef PSDN_MOCK_NEW_ZONE_H
#define PSDN_MOCK_NEW_ZONE_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "poseidon/zone/new_zone.h"

namespace poseidon {
 class MockNewZone : public NewZone {
  public:
   MockNewZone() = default;
   ~MockNewZone() override = default;
   MOCK_METHOD(Pointer*, TryAllocatePointer, (const ObjectSize), (override));
   MOCK_METHOD(uword, TryAllocateBytes, (const ObjectSize), (override));
   MOCK_METHOD(uword, TryAllocateClassBytes, (Class*), (override));
   MOCK_METHOD(void, Clear, (), (override));
   MOCK_METHOD(void, SwapSpaces, (), (override));
   MOCK_METHOD(bool, IsEmpty, (), (const override));
 };
}

#endif //PSDN_MOCK_NEW_ZONE_H