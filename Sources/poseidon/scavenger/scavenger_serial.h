#ifndef POSEIDON_SERIAL_SCAVENGER_H
#define POSEIDON_SERIAL_SCAVENGER_H

#include "poseidon/scavenger/scavenger_visitor.h"

namespace poseidon {
 class SerialScavenger : public ScavengerVisitor<false> {
   friend class Scavenger;
   friend class SerialScavengerTest;
  protected:
   explicit SerialScavenger(Scavenger* scavenger, NewZone* new_zone, OldZone* old_zone):
     ScavengerVisitor<false>(scavenger, new_zone, old_zone) { }

   void SwapSpaces();
   bool ProcessAll();
   bool ProcessRoots();
   bool ProcessToSpace();
   bool UpdateRoots();

   bool Visit(Pointer* vis) override;
  public:
   ~SerialScavenger() override = default;

   void ScavengeMemory() override;
 };
}

#endif // POSEIDON_SERIAL_SCAVENGER_H