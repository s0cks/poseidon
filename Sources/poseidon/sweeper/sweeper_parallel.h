#ifndef POSEIDON_PARALLEL_SWEEPER_H
#define POSEIDON_PARALLEL_SWEEPER_H

#include "poseidon/wsq.h"
#include "poseidon/sweeper/sweeper_base.h"

namespace poseidon {
// class ParallelSweeperTask : public Task{
//   friend class ParallelSweeperTask;
//  private:
//   WorkStealingQueue<uword>* work_;
//   FreeList* free_list_;
//
//   inline FreeList* free_list() const{
//     return free_list_;
//   }
//
//   inline bool HasWork(){
//     return !work_->empty();
//   }
//
//   inline uword GetNext(){
//     return work_->Steal();
//   }
//  public:
//   explicit ParallelSweeperTask(ParallelSweeper& sweeper):
//       Task(),
//       work_(sweeper.work()),
//       free_list_(sweeper.free_list()){
//   }
//   ~ParallelSweeperTask() override = default;
//
//   const char* name() const override{
//     return "ParallelSweeperTask";
//   }
//
//   void Run() override{
//     do{
//       do{
//         uword next;
//         if((next = GetNext()) != 0){
//           Sweeper::SweepObject(free_list(), (RawObject*)next);
//         }
//       } while(HasWork());
//     } while(Sweeper::IsSweeping());
//   }
// };

 class ParallelSweeper : public SweeperVisitor<true> {
   friend class ParallelSweeperTask;
  protected:
   bool VisitPage(Page* page) override {
     NOT_IMPLEMENTED(FATAL);//TODO: implement
     return false;
   }

   bool Visit(RawObject* ptr) override {
     NOT_IMPLEMENTED(FATAL);//TODO: implement
     return false;
   }
  public:
   explicit ParallelSweeper():
       SweeperVisitor<true>() {
   }
   ~ParallelSweeper() override = default;
 };
}

#endif // POSEIDON_PARALLEL_SWEEPER_H