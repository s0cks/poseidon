#ifndef POSEIDON_SCAVENGER_H
#define POSEIDON_SCAVENGER_H

#include "poseidon/heap/heap.h"

namespace poseidon {
#ifndef UNALLOCATED
#define UNALLOCATED 0
#endif // UNALLOCATED

 class Scavenger {
   template<bool Parallel>
   friend class ScavengerVisitor;

   friend class SerialScavenger;
   friend class ParallelScavenger;
  private:
   static void SetScavenging(bool value = true);

   static inline void
   ClearScavenging() {
     return SetScavenging(false);
   }

   static bool SerialScavenge(Scavenger* scavenger);
   static bool ParallelScavenge(Scavenger* scavenger);
  protected:
   Heap* heap_;

   explicit Scavenger(Heap* heap):
    heap_(heap) {
   }

   inline Heap* heap() const {
     return heap_;
   }

   inline NewZone* new_zone() const {
     return heap()->new_zone();
   }

   inline OldZone* old_zone() const {
     return heap()->old_zone();
   }

   virtual uword Scavenge(RawObject* ptr) {
     NOT_IMPLEMENTED(FATAL); //TODO: implement
     return UNALLOCATED;
   }

   virtual uword Promote(RawObject* ptr) {
     NOT_IMPLEMENTED(FATAL); //TODO: implement
     return UNALLOCATED;
   }
  public:
   Scavenger() = delete;
   Scavenger(const Scavenger& rhs) = delete;
   virtual ~Scavenger() = default;

   Scavenger& operator=(const Scavenger& rhs) = delete;
  public:
   static bool IsScavenging();
   static bool Scavenge(Heap* heap);
 };
}

#endif // POSEIDON_SCAVENGER_H