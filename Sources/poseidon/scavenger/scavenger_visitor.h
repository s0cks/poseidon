#ifndef POSEIDON_SCAVENGER_VISITOR_H
#define POSEIDON_SCAVENGER_VISITOR_H

#include "poseidon/heap/heap.h"
#include "poseidon/scavenger/scavenger.h"

namespace poseidon {
 template<bool Parallel>
 class ScavengerVisitor : public RawObjectVisitor, public NewPageVisitor {
  protected:
   Scavenger* scavenger_;

   explicit ScavengerVisitor(Scavenger* scavenger):
    scavenger_(scavenger) {
   }

   inline Scavenger* scavenger() const {
     return scavenger_;
   }
  public:
   ~ScavengerVisitor() override = default;

   virtual inline bool IsParallel() const {
     return Parallel;
   }

   virtual bool Scavenge() = 0;
 };
}

#endif // POSEIDON_SCAVENGER_VISITOR_H