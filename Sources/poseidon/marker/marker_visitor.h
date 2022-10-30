#ifndef POSEIDON_MARKER_VISITOR_H
#define POSEIDON_MARKER_VISITOR_H

#include "poseidon/local.h"
#include "poseidon/heap/zone.h"
#include "poseidon/heap/page.h"

namespace poseidon {
 template<bool Parallel>
 class MarkerVisitor : public RawObjectVisitor {
  protected:
   MarkerVisitor() = default;
  public:
   ~MarkerVisitor() override = default;

   virtual inline bool IsParallel() const {
     return Parallel;
   }
 };
}

#endif // POSEIDON_MARKER_VISITOR_H