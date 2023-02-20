#ifndef POSEIDON_MARKER_VISITOR_H
#define POSEIDON_MARKER_VISITOR_H

#include "poseidon/zone/zone.h"
#include "poseidon/page/page.h"
#include "poseidon/marker/marker.h"

namespace poseidon {
 template<bool Parallel>
 class MarkerVisitor : public Marker {
  protected:
   Marker* marker_;

   explicit MarkerVisitor(Marker* marker):
    Marker(),
    marker_(marker) {
   }

   inline Marker* marker() const {
     return marker_;
   }

   inline void Mark(Pointer* ptr) override {
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