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
   bool NotifyLocals();
   bool ProcessToSpace();
   uword Process(RawObject* ptr);

   bool Visit(RawObject* ptr) override {
     NOT_IMPLEMENTED(FATAL);
     return false;
   }

   bool Visit(NewPage* page) override {
     NOT_IMPLEMENTED(FATAL);
     return false;
   }
  public:
   ~SerialScavenger() override = default;

   bool Scavenge() override;
 };
}

#endif // POSEIDON_SERIAL_SCAVENGER_H