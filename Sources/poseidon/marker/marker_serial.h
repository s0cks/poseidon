#ifndef POSEIDON_SERIAL_MARKER_H
#define POSEIDON_SERIAL_MARKER_H

#include "poseidon/wsq.h"
#include "poseidon/marker/marker_visitor.h"

namespace poseidon {
 static constexpr const int64_t kDefaultSerialMarkerWorkQueueSize = 1024;
 class SerialMarker : public MarkerVisitor<false> {
  protected:
   WorkStealingQueue<uword> work_;

   static inline void Mark(RawObject* ptr) {
     ptr->SetMarkedBit();
   }

   bool Visit(RawObject* ptr) override;
  public:
   SerialMarker():
    MarkerVisitor<false>(),
    work_(kDefaultSerialMarkerWorkQueueSize) {
   }
   ~SerialMarker() override = default;

   bool MarkAllRoots();
   bool MarkAllNewRoots();
   bool MarkAllOldRoots();
 };
}

#endif // POSEIDON_SERIAL_MARKER_H