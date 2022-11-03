#include "poseidon/scavenger/scavenger.h"
#include "poseidon/scavenger/scavenger_serial.h"

namespace poseidon {
 static RelaxedAtomic<bool> scavenging_(false);

 bool Scavenger::IsScavenging() {
   return (bool)scavenging_;
 }

 void Scavenger::SetScavenging(bool value){
   scavenging_ = value;
 }

#define ALREADY_SCAVENGING { \
    DLOG(WARNING) << "already scavenging, skipping."; \
    return false;             \
}

 bool Scavenger::SerialScavenge(Scavenger* scavenger){
   if(IsScavenging())
     ALREADY_SCAVENGING;

   SetScavenging();
   SerialScavenger serial_scavenger(scavenger);
   TIMED_SECTION("SerialScavenge", {
     serial_scavenger.Scavenge();
   });
   ClearScavenging();
   return true;
 }

 bool Scavenger::ParallelScavenge(Scavenger* scavenger){
   NOT_IMPLEMENTED(FATAL); //TODO: implement
   return false;
 }

 bool Scavenger::Scavenge(Heap* heap){
   if(IsScavenging())
     ALREADY_SCAVENGING;

   Scavenger scavenger(heap);
   return HasWorkers() ?
          SerialScavenge(&scavenger) :
          ParallelScavenge(&scavenger);
 }
}