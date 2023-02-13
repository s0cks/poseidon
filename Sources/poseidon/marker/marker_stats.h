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

   MarkerStats(const PointerCounter& visited,
               const PointerCounter& marked):
               visited_(visited),
               marked_(marked) {
   }

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

   PointerCounter visited() const {
     return visited_;
   }

   PointerCounter marked() const {
     return marked_;
   }

   MarkerStats& operator=(const MarkerStats& rhs) = default;

   friend std::ostream& operator<<(std::ostream& stream, const MarkerStats& value) {
     stream << "MarkerStats(";
     stream << "visited=" << value.visited() << ", ";
     stream << "marked=" << value.marked();
     stream << ")";
     return stream;
   }
 };
}

#endif // POSEIDON_MARKER_STATS_H