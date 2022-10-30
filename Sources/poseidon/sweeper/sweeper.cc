#include "poseidon/sweeper/sweeper.h"
#include "poseidon/sweeper/sweeper_serial.h"

namespace poseidon {
 static RelaxedAtomic<bool> sweeping_(false);

 bool Sweeper::IsSweeping() {
   return (bool)sweeping_;
 }

 void Sweeper::SetSweeping(bool value){
   sweeping_ = value;
 }

 bool Sweeper::SweepObject(FreeList* free_list, RawObject* raw) { //TODO: cleanup
   DLOG(INFO) << "sweeping " << (*raw);
   NOT_IMPLEMENTED(FATAL); //TODO: implement
   return false;
   //return free_list->Insert(raw->GetStartingAddress(), raw->GetTotalSize());
 }

 bool Sweeper::SerialSweep(OldZone* zone){
   if(IsSweeping()) {
     DLOG(WARNING) << "already sweeping";
     return false;
   }

   SetSweeping();
   SerialSweeper sweeper;
   TIMED_SECTION("SerialSweep", {
     sweeper.Sweep(zone);
   });
   ClearSweeping();
   return true;
 }

 bool Sweeper::ParallelSweep(OldZone* zone){
   NOT_IMPLEMENTED(ERROR); //TODO: implement
   return false;
 }
}