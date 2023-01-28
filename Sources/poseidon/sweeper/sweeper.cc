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

 bool Sweeper::Sweep(Pointer* raw) { //TODO: cleanup
   NOT_IMPLEMENTED(FATAL); //TODO: implement
   return false;
   //return free_list->Insert(raw->GetStartingAddress(), raw->GetTotalSize());
 }

 bool Sweeper::SerialSweep(Sweeper* sweeper, FreeList* free_list){
   if(IsSweeping()) {
     DLOG(WARNING) << "already sweeping";
     return false;
   }

   SetSweeping();
   SerialSweeper serial_sweeper(sweeper, free_list);
   TIMED_SECTION("SerialSweep", {
     serial_sweeper.Sweep();
   });
   ClearSweeping();
   return true;
 }

 bool Sweeper::ParallelSweep(Sweeper* sweeper, FreeList* free_list){
   NOT_IMPLEMENTED(ERROR); //TODO: implement
   return false;
 }

 bool Sweeper::Sweep(FreeList* free_list) {
   if(IsSweeping()) {
     DLOG(WARNING) << "sweep called while already sweeping, skipping.";
     return false;
   }

   Sweeper sweeper;
   return flags::FLAGS_num_workers > 0 ?
          ParallelSweep(&sweeper, free_list) :
          SerialSweep(&sweeper, free_list);
 }
}