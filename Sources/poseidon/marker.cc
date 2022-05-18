#include "poseidon/flags.h"
#include "poseidon/local.h"
#include "poseidon/marker.h"
#include "poseidon/runtime.h"
#include "poseidon/task_pool.h"
#include "poseidon/raw_object.h"

namespace poseidon{
 static RelaxedAtomic<bool> marking(false);

 bool Marker::IsMarking(){
   return (bool)marking;
 }

 void Marker::SetMarking(){
   marking = true;
 }

 void Marker::ClearMarking(){
   marking = false;
 }

 template<bool Parallel>
 class MarkerVisitorBase : public RawObjectPointerVisitor{
  protected:
   MarkerVisitorBase() = default;
  public:
   ~MarkerVisitorBase() override = default;

   bool IsParallel() const{
     return Parallel;
   }
 };

 class SerialMarkerVisitor : public MarkerVisitorBase<false>{
  protected:
   void MarkRoots(){
     auto page = LocalPage::GetLocalPageForCurrentThread();
     while(page != nullptr){
       page->VisitPointers(this);
       page = page->GetNext();
     }
   }
  public:
   SerialMarkerVisitor() = default;
   ~SerialMarkerVisitor() override = default;

   bool Visit(RawObject** ptr) override{
     auto old_val = (*ptr);
     if(old_val->IsOld() && !old_val->IsMarked()){
       old_val->SetMarkedBit();
       DLOG(INFO) << "marked " << old_val->ToString();
     }
     //TODO: mark references?
     return true;
   }

   void MarkAll(){
     MarkRoots();
   }
 };

 class ParallelMarker;
 class ParallelMarkerTask : public Task{
   friend class ParallelMarker;
  private:
   WorkStealingQueue<uword>* work_;

   inline WorkStealingQueue<uword>* work() const{
     return work_;
   }

   inline bool HasWork() const;
   inline uword GetNext();
  public:
   explicit ParallelMarkerTask(WorkStealingQueue<uword>* work):
    Task(),
    work_(work){
   }
   ~ParallelMarkerTask() override = default;

   const char* name() const override{
     return "ParallelMarkerTask";
   }

   void Run() override{
     do{
       do{
         uword next;
         if((next = GetNext()) != 0){
           auto ptr = (RawObject*)next;
           ptr->SetMarkedBit();
         }
       } while(HasWork());
     } while(Marker::IsMarking());
   }
 };

 class ParallelMarker : public MarkerVisitorBase<true>{
   friend class ParallelMarkerTask;
  private:
   WorkStealingQueue<uword>* work_;

   void MarkLocals(){
     TIMED_SECTION("MarkLocals", {
       auto page = LocalPage::GetLocalPageForCurrentThread();
       while(page != nullptr){
         page->VisitPointers(this);
         page = page->GetNext();
       }
     });
   }

   void MarkRoots(){
     TIMED_SECTION("MarkRoots", {
       MarkLocals();

       // wait for work to finish.
       while(!work_->empty());//spin
     });
   }
  public:
   explicit ParallelMarker(WorkStealingQueue<uword>* work):
    MarkerVisitorBase<true>(),
    work_(work){
   }
   ~ParallelMarker() override = default;

   bool Visit(RawObject** ptr) override{
     auto old_val = (*ptr);
     if(!old_val->IsMarked() && !old_val->IsForwarding())
       work_->Push(old_val->GetAddress());
     return true;
   }

   void MarkAll(){
     MarkRoots();
   }
 };

 uword ParallelMarkerTask::GetNext(){
   return work()->Steal();
 }

 bool ParallelMarkerTask::HasWork() const{
   return !work()->empty();
 }

 void Marker::SerialMark(){
   SerialMarkerVisitor marker;
   TIMED_SECTION("SerialMark", {
     marker.MarkAll();
   });
 }

 void Marker::ParallelMark(){
   auto wrk = new WorkStealingQueue<uword>(2048);
   ParallelMarker marker(wrk);
   Runtime::GetTaskPool()->SubmitToAll<ParallelMarkerTask>(wrk);
   TIMED_SECTION("ParallelMark", {
     marker.MarkAll();
   });
 }

 void Marker::MarkAllLiveObjects(){
   if(IsMarking()){
     DLOG(WARNING) << "already marking.";
     return;
   }

   SetMarking();
   if(ShouldUseParallelMark()){
     ParallelMark();
   } else{
     SerialMark();
   }
   ClearMarking();
 }
}