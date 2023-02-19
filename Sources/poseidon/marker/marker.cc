#include "poseidon/flags.h"
#include "poseidon/marker/marker.h"

namespace poseidon {
 static PointerCounter marked_;
 static PointerCounter visited_;
 static RelaxedAtomic<Marker::State> state_;

 void Marker::Mark(Pointer* ptr) {
   visited_ += ptr;
   if(ptr->IsMarked())
     return;

   DLOG(INFO) << "marking " << (*ptr);
   ptr->SetMarked();
   marked_ += ptr;
   LOG_IF(WARNING, !ptr->VisitPointers(this)) << "failed to visit pointers in " << (*ptr);
 }

 void Marker::ClearStats() {
   visited_.clear();
   marked_.clear();
 }

 MarkerStats Marker::GetStats() {
   return { visited_, marked_ };
 }

 void Marker::SetState(const State& state) {
   state_ = state;
 }

 Marker::State Marker::GetState() {
   return (Marker::State)state_;
 }
}