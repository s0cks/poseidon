#ifndef POSEIDON_SCAVENGER_VISITOR_H
#define POSEIDON_SCAVENGER_VISITOR_H

#include "poseidon/heap/heap.h"
#include "poseidon/scavenger/scavenger.h"

namespace poseidon {
 template<bool Parallel>
 class ScavengerVisitor : public Scavenger {
  protected:
   Scavenger* scavenger_;

   explicit ScavengerVisitor(Scavenger* scavenger,
                             NewZone* new_zone,
                             OldZone* old_zone):
    Scavenger(new_zone, old_zone),
    scavenger_(scavenger) {
   }

   inline Scavenger* scavenger() const {
     return scavenger_;
   }
  public:
   ScavengerVisitor() = default;
   ~ScavengerVisitor() override = default;

   virtual inline bool IsParallel() const {
     return Parallel;
   }

   uword Scavenge(Pointer* ptr) override {
     return scavenger()->Scavenge(ptr);
   }

   uword Promote(Pointer* ptr) override {
     return scavenger()->Promote(ptr);
   }

   virtual void ScavengeMemory() = 0;
 };
}

#endif // POSEIDON_SCAVENGER_VISITOR_H