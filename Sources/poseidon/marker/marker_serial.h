#ifndef POSEIDON_SERIAL_MARKER_H
#define POSEIDON_SERIAL_MARKER_H

#include "poseidon/wsq.h"
#include "poseidon/marker/marker_visitor.h"

namespace poseidon {
 static constexpr const int64_t kDefaultSerialMarkerWorkQueueSize = 1024;

 class Marker;
 class SerialMarker : public MarkerVisitor<false> {
  protected:
   WorkStealingQueue<uword> work_;

   bool Visit(Pointer* ptr) override;
  public:
   explicit SerialMarker(Marker* marker):
    MarkerVisitor<false>(marker),
    work_(kDefaultSerialMarkerWorkQueueSize) {
   }
   ~SerialMarker() override = default;

   bool MarkAllRoots();
   bool MarkAllNewRoots();
   bool MarkAllOldRoots();
 };
}

#endif // POSEIDON_SERIAL_MARKER_H