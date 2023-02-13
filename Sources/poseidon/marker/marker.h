#ifndef POSEIDON_MARKER_H
#define POSEIDON_MARKER_H

#include "poseidon/heap/zone/new_zone.h"
#include "poseidon/heap/zone/old_zone.h"
#include "poseidon/marker/marker_stats.h"

namespace poseidon {
 class Marker : public RawObjectVisitor {
   template<bool IsParallel>
   friend class MarkerVisitor;
  protected:
   virtual void Mark(Pointer* ptr);

   static void ClearStats();
  public:
   Marker() = default;
   ~Marker() override = default;

   bool Visit(Pointer* ptr) override {
     if(ptr->IsMarked())
       return false;

     Mark(ptr);
     return true;
   }

  public:
   static MarkerStats GetStats();
 };
}

#endif // POSEIDON_MARKER_H