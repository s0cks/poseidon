#include "poseidon/flags.h"
#include "poseidon/marker/marker.h"
#include "poseidon/relaxed_atomic.h"
#include "poseidon/marker/marker_serial.h"

namespace poseidon {
 static RelaxedAtomic<bool> marking_(false);

 bool Marker::IsMarking() {
   return (bool)marking_;
 }

 void Marker::SetMarking(const bool value) {
   marking_ = value;
 }

 bool Marker::SerialMark(Marker* marker){
   if(IsMarking()) {
     DLOG(WARNING) << "already marking, skipping.";
     return false;
   }

   SetMarking();
   SerialMarker serial_marker(marker);
   TIMED_SECTION("SerialMark", {
     serial_marker.MarkAllRoots();
   });
   ClearMarking();
   return true;
 }

 bool Marker::ParallelMark(Marker* marker){
   NOT_IMPLEMENTED(FATAL); //TODO: implement
   return false;
 }
}