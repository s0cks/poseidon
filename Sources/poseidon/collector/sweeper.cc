#include "poseidon/wsq.h"
#include "poseidon/task_pool.h"
#include "poseidon/heap/heap.h"
#include "poseidon/heap/old_zone.h"

#include "poseidon/collector/sweeper.h"
#include "poseidon/sweeper/sweeper_serial.h"
#include "poseidon/sweeper/sweeper_parallel.h"

namespace poseidon{
 static RelaxedAtomic<bool> sweeping(false);

 static RelaxedAtomic<Timestamp> last_sweep_ts;
 static RelaxedAtomic<int64_t> last_sweep_duration_ms;
 static RelaxedAtomic<double> last_sweep_frag_perc(0.0);
 static RelaxedAtomic<int64_t> last_sweep_num(0);
 static RelaxedAtomic<int64_t> last_sweep_bytes(0);

 void Sweeper::SweepObject(FreeList* free_list, RawObject* ptr){
   if(ptr->IsMarked())
     return;

   DLOG(INFO) << "sweeping " << ptr->ToString();
   //TODO: free_list->Add(ptr->GetStartingAddress(), ptr->GetTotalSize());
#ifdef PSDN_DEBUG
   memset(ptr->GetPointer(), 0, ptr->GetPointerSize());
#endif//PSDN_DEBUG
   //TODO: update ptr ObjectTag

   last_sweep_num += 1;
   last_sweep_bytes += ptr->GetTotalSize();

   ptr->SetPointerSize(0);
 }

 bool Sweeper::IsSweeping(){
   return (bool)sweeping;
 }

 void Sweeper::SetSweeping(bool active){
   if(active){
     last_sweep_ts = Clock::now();
     sweeping = true;
     last_sweep_num = 0;
     last_sweep_bytes = 0;
   } else{
     sweeping = false;
     last_sweep_duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - GetLastSweepTimestamp()).count();
   }
 }

 Timestamp Sweeper::GetLastSweepTimestamp(){
   return (Timestamp)last_sweep_ts;
 }

 int64_t Sweeper::GetLastSweepDurationMillis(){
   return (int64_t)last_sweep_duration_ms;
 }

 double Sweeper::GetLastSweepFragmentationPercentage(){
   return (double)last_sweep_frag_perc;
 }

 int64_t Sweeper::GetNumberOfObjectsLastSweep(){
   return (int64_t)last_sweep_num;
 }

 int64_t Sweeper::GetNumberOfBytesLastSweep(){
   return (int64_t)last_sweep_bytes;
 }

 static inline double
 GetPercentageFreeInFreeList(OldZone* zone){
   return 0; //TODO: GetPercentageOf(zone->free_list()->GetTotalBytesFree(), zone->GetSize());
 }

 void Sweeper::SerialSweep(OldZone* old_zone){
   SerialSweeper sweeper;
   TIMED_SECTION("SerialSweep", {
     old_zone->VisitPages(&sweeper);
   });
 }

 void Sweeper::ParallelSweep(OldZone* old_zone){
   NOT_IMPLEMENTED(FATAL);//TODO: implement
//   ParallelSweeper sweeper;
//   Runtime::GetTaskPool()->SubmitToAll<ParallelSweeperTask>(sweeper);
//   TIMED_SECTION("ParallelSweep", {
//     sweeper.Sweep();
//   });
 }

 void Sweeper::Sweep(){
   if(IsSweeping()){
     DLOG(WARNING) << "already sweeping.";
     return;
   }

   auto heap = Heap::GetCurrentThreadHeap();
   auto old_zone = heap->old_zone();
   DLOG(INFO) << "sweeping " << (old_zone);
   DLOG(INFO) << "sweeper stats (before): " << GetStats();

   double perc_free_before = GetPercentageFreeInFreeList(&old_zone);
   SetSweeping();
   if(HasWorkers()){
     ParallelSweep(&old_zone);
   } else{
     SerialSweep(&old_zone);
   }
   ClearSweeping();
   double perc_free_after = GetPercentageFreeInFreeList(&old_zone);
   last_sweep_frag_perc = perc_free_after - perc_free_before;
   DLOG(INFO) << "sweeper stats (after): " << GetStats();
 }
}
