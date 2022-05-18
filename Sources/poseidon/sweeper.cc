#include "poseidon/wsq.h"
#include "poseidon/sweeper.h"
#include "poseidon/runtime.h"
#include "poseidon/allocator.h"
#include "poseidon/task_pool.h"

namespace poseidon{
 static RelaxedAtomic<bool> sweeping(false);

 static RelaxedAtomic<Timestamp> last_sweep_ts;
 static RelaxedAtomic<int64_t> last_sweep_duration_ms;
 static RelaxedAtomic<double> last_sweep_frag_perc(0.0);
 static RelaxedAtomic<int64_t> last_sweep_num(0);
 static RelaxedAtomic<int64_t> last_sweep_bytes(0);

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

 template<bool Parallel>
 class SweeperVisitorBase : public RawObjectPointerVisitor{
  protected:
   SweeperVisitorBase() = default;
  public:
   ~SweeperVisitorBase() override = default;

   inline bool IsParallel() const{
     return Parallel;
   }
 };

 class SerialSweeper : public SweeperVisitorBase<false>{
  protected:
   FreeList* free_list_;
  public:
   explicit SerialSweeper() = default;
   ~SerialSweeper() override = default;

   bool Visit(RawObject** ptr) override{
     return true;
   }
 };

 class ParallelSweeper : public SweeperVisitorBase<true>{
   friend class ParallelSweeperTask;
  protected:
   WorkStealingQueue<uword>* work_;
   FreeList* free_list_;
  public:
   explicit ParallelSweeper(OldZone* zone):
    SweeperVisitorBase<true>(),
    work_(new WorkStealingQueue<uword>(4096)),
    free_list_(&zone->free_list_){
   }
   ~ParallelSweeper() override = default;//TODO: free work_

   inline FreeList* free_list() const{
     return free_list_;
   }

   WorkStealingQueue<uword>* work() const{
     return work_;
   }

   bool Visit(RawObject** ptr) override{
     return true;
   }
 };

 class ParallelSweeperTask : public Task{
   friend class ParallelSweeperTask;
  private:
   WorkStealingQueue<uword>* work_;
   FreeList* free_list_;

   inline bool HasWork(){
     return !work_->empty();
   }

   inline uword GetNext(){
     return work_->Steal();
   }

   void SweepObject(RawObject* ptr){
     DLOG(INFO) << "sweeping " << ptr->ToString() << "....";
     //TODO: update object tag
#ifdef PSDN_DEBUG
     memset(ptr->GetPointer(), 0, ptr->GetPointerSize());
#endif//PSDN_DEBUG
     free_list_->Add(ptr->GetAddress(), ptr->GetTotalSize());

     last_sweep_num += 1;
     last_sweep_bytes += ptr->GetTotalSize();//TODO: should we use total size or pointer size
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
           SweepObject((RawObject*)next);
         }
       } while(HasWork());
     } while(Sweeper::IsSweeping());
   }
 };

 void Sweeper::SerialSweep(OldZone* old_zone){
   SerialSweeper sweeper;
   TIMED_SECTION("SerialSweep", {

   });
 }

 static inline double
 GetPercentageFreeInFreeList(OldZone* zone){
   return GetPercentageOf(zone->free_list()->GetTotalBytesFree(), zone->GetSize());
 }

 void Sweeper::ParallelSweep(OldZone* old_zone){
   double perc_free_before = GetPercentageFreeInFreeList(old_zone);

   ParallelSweeper sweeper(old_zone);
   Runtime::GetTaskPool()->SubmitToAll<ParallelSweeperTask>(sweeper);
   TIMED_SECTION("ParallelSweep", {
     old_zone->VisitAllPages([&](OldPage* page){//TODO: should we be visiting the marked pages, or every page?
       page->VisitPointers([&](RawObject* raw){
         if(!raw->IsMarked())
           sweeper.work()->Push(raw->GetAddress());
         return true;
       });
       return true;
     });
   });

   double perc_free_after = GetPercentageFreeInFreeList(old_zone);
   last_sweep_frag_perc = perc_free_after - perc_free_before;
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

   SetSweeping();
   if(ShouldUseParallelSweep()){
     ParallelSweep(old_zone);
   } else{
     SerialSweep(old_zone);
   }
   ClearSweeping();
   DLOG(INFO) << "sweeper stats (after): " << GetStats();
 }
}