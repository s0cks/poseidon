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
   WorkStealingQueue<uword> work_;
  public:
   ParallelSweeper():
    SweeperVisitorBase<true>(),
    work_(4096){
   }
   ~ParallelSweeper() override = default;

   bool Visit(RawObject** ptr) override{
     return true;
   }
 };

 class ParallelSweeperTask : public Task{
   friend class ParallelSweeperTask;
  private:
   ParallelSweeper* sweeper_;
   WorkStealingQueue<uword>* work_;

   inline ParallelSweeper* sweeper() const{
     return sweeper_;
   }

   inline bool HasWork(){
     return !work_->empty();
   }

   inline uword GetNext(){
     return work_->Steal();
   }
  public:
   explicit ParallelSweeperTask(ParallelSweeper* sweeper):
    Task(),
    sweeper_(sweeper),
    work_(&sweeper->work_){
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
           auto old_val = (RawObject*)next;
           DLOG(INFO) << "sweeping " << old_val->ToString() << "....";
           //TODO: process object
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
   ParallelSweeper sweeper;
   Runtime::GetTaskPool()->SubmitToAll<ParallelSweeperTask>(&sweeper);
   TIMED_SECTION("ParallelSweep", {
     old_zone.VisitPages([&](OldPage* page){//TODO: should we be visiting the marked pages, or every page?
       DLOG(INFO) << "visiting " << (*page) << "....";
       page->VisitPointers([&](RawObject* raw){
         if(!raw->IsMarked()){
           DLOG(INFO) << "sweeping " << raw->ToString();
         }
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