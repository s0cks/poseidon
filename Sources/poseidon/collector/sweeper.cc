#include "poseidon/wsq.h"
#include "poseidon/runtime.h"
#include "poseidon/task_pool.h"
#include "poseidon/heap/heap.h"
#include "poseidon/heap/old_zone.h"
#include "poseidon/collector/sweeper.h"

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
   free_list->Add(ptr->GetAddress(), ptr->GetTotalSize());
#ifdef PSDN_DEBUG
   memset(ptr->GetPointer(), 0, ptr->GetPointerSize());
#endif//PSDN_DEBUG
   //TODO: update ptr ObjectTag

   last_sweep_num += 1;
   last_sweep_bytes += ptr->GetTotalSize();

   ptr->SetPointerSize(0);
   DLOG(INFO) << "swept " << ptr->ToString() << "....";
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

 class ParallelSweeper : public SweeperVisitorBase<true>{
   friend class ParallelSweeperTask;
  protected:
   WorkStealingQueue<uword>* work_;
   FreeList* free_list_;
  public:
   explicit ParallelSweeper(OldZone* zone):
    SweeperVisitorBase<true>(zone),
    work_(new WorkStealingQueue<uword>(1024)),
    free_list_(zone->free_list()){
   }
   ~ParallelSweeper() override = default;//TODO: free work_

   inline FreeList* free_list() const{
     return free_list_;
   }

   WorkStealingQueue<uword>* work() const{
     return work_;
   }

   bool Visit(RawObject* ptr) override{
     if(!ptr->IsMarked())
       work()->Push(ptr->GetAddress());
     return true;
   }

   void Sweep() override{
     zone()->VisitPages([&](OldPage* page){
       page->VisitPointers(this);
       return true;
     });
   }
 };

 class ParallelSweeperTask : public Task{
   friend class ParallelSweeperTask;
  private:
   WorkStealingQueue<uword>* work_;
   FreeList* free_list_;

   inline FreeList* free_list() const{
     return free_list_;
   }

   inline bool HasWork(){
     return !work_->empty();
   }

   inline uword GetNext(){
     return work_->Steal();
   }
  public:
   explicit ParallelSweeperTask(ParallelSweeper& sweeper):
    Task(),
    work_(sweeper.work()),
    free_list_(sweeper.free_list()){
   }
   ~ParallelSweeperTask() override = default;

   const char* name() const override{
     return "ParallelSweeperTask";
   }

   void Run() override{
     do{
       do{
         uword next;
         if((next = GetNext()) != 0){
           Sweeper::SweepObject(free_list(), (RawObject*)next);
         }
       } while(HasWork());
     } while(Sweeper::IsSweeping());
   }
 };

 static inline double
 GetPercentageFreeInFreeList(OldZone* zone){
   return GetPercentageOf(zone->free_list()->GetTotalBytesFree(), zone->GetSize());
 }

 void Sweeper::SerialSweep(OldZone* old_zone){
   SerialSweeper sweeper(old_zone);
   TIMED_SECTION("SerialSweep", {
     sweeper.Sweep();
   });
 }

 void Sweeper::ParallelSweep(OldZone* old_zone){
   ParallelSweeper sweeper(old_zone);
   Runtime::GetTaskPool()->SubmitToAll<ParallelSweeperTask>(sweeper);
   TIMED_SECTION("ParallelSweep", {
     sweeper.Sweep();
   });
 }

 void Sweeper::Sweep(){
   if(IsSweeping()){
     DLOG(WARNING) << "already sweeping.";
     return;
   }

   auto heap = Heap::GetCurrentThreadHeap();
   auto old_zone = heap->old_zone();
   DLOG(INFO) << "sweeping " << (*old_zone);
   DLOG(INFO) << "sweeper stats (before): " << GetStats();

   double perc_free_before = GetPercentageFreeInFreeList(old_zone);
   SetSweeping();
   if(HasWorkers()){
     ParallelSweep(old_zone);
   } else{
     SerialSweep(old_zone);
   }
   ClearSweeping();
   double perc_free_after = GetPercentageFreeInFreeList(old_zone);
   last_sweep_frag_perc = perc_free_after - perc_free_before;
   DLOG(INFO) << "sweeper stats (after): " << GetStats();
 }

 bool SerialSweeper::Visit(RawObject* ptr){
   DLOG(INFO) << "checking: " << ptr->ToString();
   if(ptr->IsMarked())
     return true; // don't sweep marked objects
   Sweeper::SweepObject(free_list(), ptr);
   return true;
 }
}
