#ifndef POSEIDON_MARKER_STATS_H
#define POSEIDON_MARKER_STATS_H

#include <ostream>
#include "poseidon/pointer_counter.h"

namespace poseidon {
 class MarkerStats {
   friend class Marker;
  protected:
   PointerCounter visited_;
   PointerCounter marked_;

   PointerCounter& visited() {
     return visited_;
   }

   PointerCounter& marked() {
     return marked_;
   }
  public:
   MarkerStats() = default;
   MarkerStats(const MarkerStats& rhs) = default;
   ~MarkerStats() = default;

   MarkerStats& operator=(const MarkerStats& rhs) = default;

   friend std::ostream& operator<<(std::ostream& stream, const MarkerStats& value) {
     NOT_IMPLEMENTED(FATAL);
     return stream;
   }
 };
}

#endif // POSEIDON_MARKER_STATS_H