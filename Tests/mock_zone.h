#ifndef POSEIDON_MOCK_ZONE_H
#define POSEIDON_MOCK_ZONE_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <glog/logging.h>

#include "poseidon/zone.h"

namespace poseidon{
 class MockMemoryRegion : public MemoryRegion{
  public:
   MockMemoryRegion() = default;
   ~MockMemoryRegion() override = default;

   MOCK_METHOD(bool, Protect, (const ProtectionMode&), (const));
 };

 class MockZone : public Zone{
  public:
   MockZone() = default;
   ~MockZone() override = default;

   MOCK_METHOD(bool, Contains, (uword), (const));
   MOCK_METHOD(void, SwapSpaces, (), ());
   MOCK_METHOD(void, ClearZone, (), (const));
   MOCK_METHOD(uword, Allocate, (int64_t), ());
 };

 class MockNewZone : public NewZone{
  public:
   MockNewZone() = default;
   ~MockNewZone() override = default;

   MOCK_METHOD(bool, Contains, (uword), (const));
   MOCK_METHOD(void, SwapSpaces, (), ());
   MOCK_METHOD(void, ClearZone, (), (const));
   MOCK_METHOD(uword, Allocate, (int64_t), ());
 };

 class MockOldZone : public OldZone{
  public:
   MockOldZone() = default;
   ~MockOldZone() override = default;

   MOCK_METHOD(bool, Contains, (uword), (const));
   MOCK_METHOD(void, SwapSpaces, (), ());
   MOCK_METHOD(void, ClearZone, (), (const));
   MOCK_METHOD(uword, Allocate, (int64_t), ());
 };
}

#endif //POSEIDON_MOCK_ZONE_H