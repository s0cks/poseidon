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
       return page->VisitPointers(this);
     });
   }

   bool Mark(RawObject* raw) override {
     TIMED_SECTION("Mark", {
       raw->SetMarkedBit();
       return raw->IsMarked();
     });
   }
 };
}

#endif // POSEIDON_SERIAL_MARKER_H