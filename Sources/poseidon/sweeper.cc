#include "poseidon/wsq.h"
#include "poseidon/sweeper.h"
#include "poseidon/runtime.h"
#include "poseidon/allocator.h"
#include "poseidon/task_pool.h"

namespace poseidon{
 static RelaxedAtomic<bool> sweeping(false);

 bool Sweeper::IsSweeping(){
   return (bool)sweeping;
 }

 void Sweeper::SetSweeping(){
   sweeping = true;
 }

 void Sweeper::ClearSweeping(){
   sweeping = false;
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
     GCLOG(1) << "sweeping " << ptr->ToString() << "....";
     //TODO: update object tag
#ifdef PSDN_DEBUG
     memset(ptr->GetPointer(), 0, ptr->GetPointerSize());
#endif//PSDN_DEBUG
     free_list_->Add(ptr->GetAddress(), ptr->GetTotalSize());
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

 void Sweeper::SerialSweep(){
   auto heap = Heap::GetCurrentThreadHeap();
   auto old_zone = heap->old_zone();

   SerialSweeper sweeper;
   TIMED_SECTION("SerialSweep", {

   });
 }

 void Sweeper::ParallelSweep(){
   auto heap = Heap::GetCurrentThreadHeap();
   auto old_zone = heap->old_zone();
   GCLOG(1) << "sweeping " << (*old_zone);

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
 }

 void Sweeper::Sweep(){
   if(IsSweeping()){
     DLOG(WARNING) << "already sweeping.";
     return;
   }

   SetSweeping();
   if(ShouldUseParallelMark()){
     ParallelSweep();
   } else{
     SerialSweep();
   }
   ClearSweeping();
 }
}