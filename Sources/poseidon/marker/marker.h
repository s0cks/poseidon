#ifndef POSEIDON_MARKER_H
#define POSEIDON_MARKER_H

#include "poseidon/heap/new_zone.h"
#include "poseidon/heap/old_zone.h"
#include "poseidon/marker/marker_stats.h"

namespace poseidon {
 class Marker {
   template<bool Parallel>
   friend class MarkerVisitor;

   friend class SerialMarker;
   friend class ParallelMarker;
  private:
   static void SetMarking(bool value = true);

   static inline void
   ClearMarking() {
     return SetMarking(true);
   }

   static bool SerialMark(Marker* marker);
   static bool ParallelMark(Marker* marker);
  protected:
   MarkerStats stats_;

   Marker() = default;

   inline MarkerStats& stats() {
     return stats_;
   }

   virtual bool Mark(Pointer* ptr);
  public:
   Marker(const Marker& rhs) = delete;
   virtual ~Marker() = default;

   Marker& operator=(const Marker& rhs) = delete;
  public:
   static bool IsMarking();
   static bool Mark();
 };
}

#endif // POSEIDON_MARKER_H