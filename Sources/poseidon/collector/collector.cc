#include <glog/logging.h>

#include "poseidon/utils.h"
#include "poseidon/common.h"
#include "poseidon/relaxed_atomic.h"

#include "poseidon/marker/marker.h"
#include "poseidon/sweeper/sweeper.h"
#include "poseidon/collector/compactor.h"
#include "poseidon/collector/scavenger.h"
#include "poseidon/collector/collector.h"

namespace poseidon{
 static RelaxedAtomic<Collector::State> state(Collector::kIdle);

 Collector::State Collector::GetState(){
   return (Collector::State)state;
 }

 void Collector::SetState(State s){
   state = s;
 }

 void Collector::MinorCollection(){
   if(IsMinorCollection()){
     LOG(ERROR) << "minor collection is already running, skipping new minor collection.";
     return;
   }

   if(IsMajorCollection()){
     LOG(WARNING) << "major collection is running, skipping new minor collection.";
     return;
   }

   TIMED_SECTION("MinorCollection", {
     //TODO: Scavenger::Scavenge();
   });
 }

 void Collector::MajorCollection(){//TODO: decide between sweeper & compactor
   if(IsMajorCollection()){
     LOG(ERROR) << "major collection is already running, skipping new major collection.";
     return;
   }

   if(IsMinorCollection()){
     LOG(WARNING) << "minor collection is already running, skipping new major collection.";
     return;
   }

   TIMED_SECTION("MajorCollection", {
     //TODO: mark objects
     //TODO: sweep objects
     //Compactor::SerialCompact();
   });
 }
}