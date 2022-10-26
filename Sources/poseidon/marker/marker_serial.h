#ifndef POSEIDON_SERIAL_MARKER_H
#define POSEIDON_SERIAL_MARKER_H

#include "poseidon/marker/marker_visitor.h"

namespace poseidon {
 class SerialMarker : public MarkerVisitor<false> {
  public:
   SerialMarker() = default;
   ~SerialMarker() override = default;

   bool MarkPage(Page* page) override {
     TIMED_SECTION("MarkPage", {
       if(!page->VisitPointers(this))
         return false;
     });
     return true;
   }
 };
}

#endif // POSEIDON_SERIAL_MARKER_H