#ifndef POSEIDON_MARKER_H
#define POSEIDON_MARKER_H

#include "poseidon/heap/new_zone.h"
#include "poseidon/heap/old_zone.h"

namespace poseidon {
 class Page;
 class Marker {
  protected:
   static void SetMarking(bool value = true);

   static inline void
   ClearMarking() {
     return SetMarking(false);
   }

   static bool SerialMark(Section& section);
   static bool ParallelMark(Section& section);

   static inline bool
   Mark(Section& section) {
     return HasWorkers() ?
            ParallelMark(section) :
            SerialMark(section);
   }
  public:
   Marker() = delete;
   Marker(const Marker& rhs) = delete;
   ~Marker() = delete;

   static bool IsMarking();
   static bool MarkRoots();

   static inline bool
   MarkNewPage(NewPage& page) {
     TIMED_SECTION("MarkNewPage", {
       return Mark(page);
     });
   }

   static inline bool
   MarkOldPage(OldPage& page) {
     TIMED_SECTION("MarkOldPage", {
       return Mark(page);
     });
   }

   static inline bool
   MarkNewZone(NewZone& zone) {
     TIMED_SECTION("MarkNewZone", {
       return Mark(zone);
     });
   }

   static inline bool
   MarkOldZone(OldZone& zone) {
     TIMED_SECTION("MarkOldZone", {
       return Mark(zone);
     });
   }

   Marker& operator=(const Marker& rhs) = delete;
 };
}

#endif // POSEIDON_MARKER_H