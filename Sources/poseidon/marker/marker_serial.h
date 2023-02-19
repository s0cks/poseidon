#ifndef POSEIDON_SERIAL_MARKER_H
#define POSEIDON_SERIAL_MARKER_H

#include "poseidon/wsq.h"
#include "poseidon/marker/marker_visitor.h"

namespace poseidon {
 static constexpr const int64_t kDefaultSerialMarkerWorkQueueSize = 1024;

 class Marker;
 class SerialMarker : public MarkerVisitor<false> {
  protected:
   bool Visit(Pointer* ptr) override;
  public:
   explicit SerialMarker(Marker* marker):
    MarkerVisitor<false>(marker) { }
   ~SerialMarker() override = default;

   void MarkAllRoots();
 };
}

#endif // POSEIDON_SERIAL_MARKER_H