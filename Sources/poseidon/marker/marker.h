#ifndef POSEIDON_MARKER_H
#define POSEIDON_MARKER_H

#include "poseidon/zone/new_zone.h"
#include "poseidon/zone/old_zone.h"
#include "poseidon/marker/marker_stats.h"

namespace poseidon {
#define FOR_EACH_MARKER_STATE(V) \
 V(Idle)                         \
 V(MarkingRoots)                 \
 V(MarkingNewPointers)           \
 V(MarkingOldPointers)

 class Marker : public RawObjectVisitor {
  public:
   enum class State : word {
#define DEFINE_MARKER_STATE(Name) k##Name,
     FOR_EACH_MARKER_STATE(DEFINE_MARKER_STATE)
#undef DEFINE_MARKER_STATE
   };

   friend std::ostream& operator<<(std::ostream& stream, const State& state) {
     switch(state) {
#define DEFINE_TOSTRING(Name) case State::k##Name: return stream << #Name;
       FOR_EACH_MARKER_STATE(DEFINE_TOSTRING)
#undef DEFINE_TOSTRING
       default:
         return stream << "[unknown: " << static_cast<word>(state) << "]";
     }
   }

   template<bool IsParallel>
   friend class MarkerVisitor;
   friend class ParallelMarkerTask;
  protected:
   virtual void Mark(Pointer* ptr);

   static void ClearStats();
   static void SetState(const State& state);

#define DEFINE_SET_STATE(Name) \
   static inline void Set##Name() { return SetState(State::k##Name); }
   FOR_EACH_MARKER_STATE(DEFINE_SET_STATE);
#undef DEFINE_SET_STATE
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
   static State GetState();

#define DEFINE_STATE_CHECK(Name) \
   static inline bool Is##Name() { return GetState() == State::k##Name; }
   FOR_EACH_MARKER_STATE(DEFINE_STATE_CHECK)
#undef DEFINE_STATE_CHECK

   static inline bool IsMarking() {
     switch(GetState()) {
       case State::kMarkingRoots:
       case State::kMarkingNewPointers:
       case State::kMarkingOldPointers:
         return true;
       case State::kIdle:
       default:
         return false;
     }
   }
 };
}

#endif // POSEIDON_MARKER_H