#ifndef POSEIDON_SCAVENGER_H
#define POSEIDON_SCAVENGER_H

#include "poseidon/heap/heap.h"

namespace poseidon {
#define FOR_EACH_SCAVENGER_STATE(V) \
 V(Idle)                            \
 V(ProcessingRoots)                 \
 V(ProcessingToSpace)

 class Scavenger : public RawObjectVisitor {
   template<bool Parallel>
   friend class ScavengerVisitor;

   friend class SerialScavenger;
   friend class SerialScavengerTest;
   friend class ParallelScavenger;
   friend class ParallelScavengerTest;
  public:
   enum class State : word {
#define DEFINE_SCAVENGER_STATE(Name) k##Name,
     FOR_EACH_SCAVENGER_STATE(DEFINE_SCAVENGER_STATE)
#undef DEFINE_SCAVENGER_STATE
   };

   friend std::ostream& operator<<(std::ostream& stream, const State& state) {
     switch(state) {
#define DEFINE_TOSTRING(Name) case State::k##Name: return stream << #Name;
       FOR_EACH_SCAVENGER_STATE(DEFINE_TOSTRING)
#undef DEFINE_TOSTRING
       default:
         return stream << "[unknown: " << static_cast<word>(state) << "]";
     }
   }

   static void SetState(const State& state);

   static inline void ClearState() {
     return SetState(State::kIdle);
   }

#define DEFINE_SET_STATE(Name) \
   static inline void Set##Name() { return SetState(State::k##Name); }
   FOR_EACH_SCAVENGER_STATE(DEFINE_SET_STATE)
#undef DEFINE_SET_STATE
  private:
   static bool SerialScavenge(Scavenger* scavenger);
   static bool ParallelScavenge(Scavenger* scavenger);
  protected:
   NewZone* new_zone_;
   OldZone* old_zone_;
   Semispace fromspace_;
   Semispace tospace_;

   explicit Scavenger(NewZone* new_zone, OldZone* old_zone):
    new_zone_(new_zone),
    old_zone_(old_zone),
    fromspace_(new_zone->fromspace()),
    tospace_(new_zone->tospace()) {
   }

   inline Semispace* fromspace() {
     return &fromspace_;
   }

   inline Semispace* tospace() {
     return &tospace_;
   }

   inline NewZone* new_zone() const {
     return new_zone_;
   }

   inline OldZone* old_zone() const {
     return old_zone_;
   }

   virtual uword Scavenge(Pointer* ptr);
   virtual uword Promote(Pointer* ptr);

   bool Visit(Pointer* ptr) override {
     NOT_IMPLEMENTED(FATAL); //TODO: implement
     return false;
   }
  public:
   Scavenger() = default;
   ~Scavenger() override = default;
  public:
   static State GetState();

#define DEFINE_STATE_CHECK(Name) static inline bool Is##Name() { return GetState() == State::k##Name; }
   FOR_EACH_SCAVENGER_STATE(DEFINE_STATE_CHECK)
#undef DEFINE_STATE_CHECK

   static bool ScavengeMemory(Heap* heap);
 };
}

#endif // POSEIDON_SCAVENGER_H