#ifndef POSEIDON_MARKER_VISITOR_H
#define POSEIDON_MARKER_VISITOR_H

#include "poseidon/heap/zone.h"
#include "poseidon/heap/page.h"
#include "poseidon/marker/marker.h"

namespace poseidon {
 template<bool Parallel>
 class MarkerVisitor : public RawObjectVisitor {
  protected:
   Marker* marker_;

   explicit MarkerVisitor(Marker* marker):
    RawObjectVisitor(),
    marker_(marker) {
   }

   inline Marker* marker() const {
     return marker_;
   }

   inline bool Mark(RawObject* ptr) {
     return marker()->Mark(ptr);
   }
  public:
   ~MarkerVisitor() override = default;

   virtual inline bool IsParallel() const {
     return Parallel;
   }
 };
}

#endif // POSEIDON_MARKER_VISITOR_H