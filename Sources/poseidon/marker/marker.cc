#include "poseidon/flags.h"
#include "poseidon/marker/marker.h"
#include "poseidon/relaxed_atomic.h"
#include "poseidon/marker/marker_serial.h"

namespace poseidon {
 static RelaxedAtomic<bool> marking_(false);

 bool Marker::IsMarking() {
   return (bool)marking_;
 }

 void Marker::SetMarking(bool value) {
   marking_ = value;
 }

 bool Marker::SerialMark(Section& section){
   SetMarking();

   SerialMarker marker;
   TIMED_SECTION("SerialMark", {
     section.VisitPointers(&marker);
   });
   ClearMarking();
   return true; //TODO: cleanup
 }

 bool Marker::ParallelMark(Section& section){
   SetMarking();
   TIMED_SECTION("ParallelMark", {
     NOT_IMPLEMENTED(ERROR); //TODO: implement
   });
   ClearMarking();
   return true;
 }

 bool Marker::MarkRoots() {

 }
}