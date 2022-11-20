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

   friend class ScavengerTest;
   friend class SerialScavenger;
   friend class SerialScavengerTest;
   friend class ParallelScavenger;
   friend class ParallelScavengerTest;
  private:
   static void SetScavenging(bool value = true);

   static inline void
   ClearScavenging() {
     return SetScavenging(false);
   }

   static bool SerialScavenge(Scavenger* scavenger);
   static bool ParallelScavenge(Scavenger* scavenger);
  protected:
   NewZone* new_zone_;
   OldZone* old_zone_;
   Semispace fromspace_;
   Semispace tospace_;

   explicit Scavenger(Heap* heap):
    new_zone_(heap->new_zone()),
    old_zone_(heap->old_zone()),
    fromspace_(heap->new_zone()->fromspace()),
    tospace_(heap->new_zone()->tospace()) {
   }

   inline NewZone* new_zone() const {
     return new_zone_;
   }

   inline OldZone* old_zone() const {
     return old_zone_;
   }

   inline Semispace& fromspace() {
     return fromspace_;
   }

   inline Semispace& tospace() {
     return tospace_;
   }

   virtual uword Scavenge(RawObject* ptr);
   virtual uword Promote(RawObject* ptr);
   virtual uword Process(RawObject* ptr);
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