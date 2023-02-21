#ifndef POSEIDON_SERIAL_SCAVENGER_H
#define POSEIDON_SERIAL_SCAVENGER_H

#include "poseidon/scavenger/scavenger_visitor.h"

namespace poseidon {
 class SerialScavenger : public ScavengerVisitor<false> {
   friend class Scavenger;
   friend class SerialScavengerTest;
  protected:
   Array<uword> work_;

   explicit SerialScavenger(Scavenger* scavenger, NewZone* new_zone, OldZone* old_zone):
     ScavengerVisitor<false>(scavenger, new_zone, old_zone),
     work_(10) {
   }

   void SwapSpaces();
   void ProcessRoots();
   void ProcessToSpace();
   bool UpdateRoots();

   bool Visit(Pointer* vis) override;

   inline bool HasWork() const {
     return !work_.IsEmpty();
   }
  public:
   ~SerialScavenger() override = default;

   void ScavengeMemory() override;
 };
}

#endif // POSEIDON_SERIAL_SCAVENGER_H