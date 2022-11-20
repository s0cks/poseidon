#ifndef POSEIDON_SERIAL_SCAVENGER_H
#define POSEIDON_SERIAL_SCAVENGER_H

#include "poseidon/scavenger/scavenger_visitor.h"

namespace poseidon {
 class SerialScavenger : public ScavengerVisitor<false> {
   friend class Scavenger;
   friend class SerialScavengerTest;
  protected:
   explicit SerialScavenger(Scavenger* scavenger):
     ScavengerVisitor<false>(scavenger) {
   }

   void SwapSpaces();
   bool ProcessAll();
   bool ProcessRoots();
   bool ProcessToSpace();

   bool Visit(RawObject* vis) override {
     NOT_IMPLEMENTED(ERROR); //TODO: implement
     return false;
   }
  public:
   ~SerialScavenger() override = default;

   bool Scavenge() override;
 };
}

#endif // POSEIDON_SERIAL_SCAVENGER_H