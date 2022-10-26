#ifndef POSEIDON_TEST_NEW_ZONE_H
#define POSEIDON_TEST_NEW_ZONE_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "poseidon/flags.h"
#include "poseidon/heap/new_zone.h"
#include "poseidon/heap/semispace.h"

namespace poseidon{
 using namespace ::testing;

 class NewZoneTest : public Test {
  protected:
   NewZoneTest() = default;

   static inline Semispace
   GetFromspace(const NewZone& zone) {
     return zone.GetFromspace();
   }

   static inline Semispace
   GetTospace(const NewZone& zone) {
     return zone.GetTospace();
   }
  public:
   ~NewZoneTest() override = default;
 };
}

#endif//POSEIDON_TEST_NEW_ZONE_H