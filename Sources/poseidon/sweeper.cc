#include "poseidon/wsq.h"
#include "poseidon/sweeper.h"
#include "poseidon/allocator.h"

namespace poseidon{
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

 class SerialSweeperVisitor : public SweeperVisitorBase<false>{
  protected:
   OldPage* page_;
   WorkStealingQueue<uword>* work_;
  public:
   explicit SerialSweeperVisitor(OldPage* page):
    page_(page),
    work_(nullptr){
   }
   ~SerialSweeperVisitor() override = default;

   bool Visit(RawObject** ptr) override{
     return true;
   }
 };

 void Sweeper::SerialSweep(OldPage* page){
   SerialSweeperVisitor visitor(page);
//   auto locals = LocalPage::GetLocalPageForCurrentThread();
//   locals->VisitPointers([&](RawObject** ptr){
//     auto old_val = (*ptr);
//     if(old_val->IsNew() && !old_val->IsForwarding()){
//       auto new_val = (RawObject*)scavenger()->ProcessObject(old_val);
//       new_val->SetRememberedBit();
//       (*ptr) = new_val;
//     }
//     return true;
//   });
 }

 class ParallelSweeperVisitor : public SweeperVisitorBase<true>{
  protected:
   WorkStealingQueue<uword>* work_;
  public:
   ParallelSweeperVisitor() = default;
   ~ParallelSweeperVisitor() override = default;

   bool Visit(RawObject** ptr) override{
     return true;
   }
 };

 void Sweeper::Sweep(OldPage* page){
   DLOG(INFO) << "sweeping " << (*page) << "....";

   SerialSweep(page);
 }
}