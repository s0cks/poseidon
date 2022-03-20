#include <glog/logging.h>

#include "poseidon/wsq.h"
#include "poseidon/heap.h"
#include "poseidon/marker.h"
#include "poseidon/allocator.h"
#include "poseidon/task_pool.h"

namespace poseidon{
 typedef WorkStealingQueue<uword> MarkerWork;

 template<bool Parallel>
 class MarkerVisitorBase : public RawObjectVisitor{
  protected:
   Marker* marker_;

   explicit MarkerVisitorBase(Marker* marker):
    marker_(marker){
   }
  public:
   ~MarkerVisitorBase() override = default;

   bool IsParallel() const{
     return Parallel;
   }
 };

 class SerialMarkerVisitor : public MarkerVisitorBase<false>{
  public:
   explicit SerialMarkerVisitor(Marker* marker):
    MarkerVisitorBase<false>(marker){
   }
   ~SerialMarkerVisitor() override = default;

   bool Visit(RawObject* val) override{
     DLOG(INFO) << "visiting " << val->ToString() << ".....";
     return true;
   }

   template<typename T>
   void MarkLiveObjects(){
     DLOG(INFO) << "performing SerialMark....";
     T::VisitRoots(this);
   }
 };

 class ParallelMarkerVisitor;
 class ParallelMarkTask : public Task{
  private:
   Marker* marker_;
   MarkerWork* work_;
  public:
   ParallelMarkTask(Marker* marker, MarkerWork* work):
    Task(),
    marker_(marker),
    work_(work){
   }
   ~ParallelMarkTask() override = default;

   const char* name() const override{
     return "ParallelMark";
   }

   void Run() override{
     while(!work_->empty()){
       uword next;
       if((next = work_->Steal()) != 0){
         auto ptr = (RawObject*)next;
         if(ptr->GetPointerSize() > 0)
           marker_->MarkObject(ptr);
       }
     }
   }
 };

 class ParallelMarkerVisitor : public MarkerVisitorBase<true>{
   friend class ParallelMarkTask;
  protected:
   TaskPool pool_;
   WorkStealingQueue<uword> work_;
  public:
   explicit ParallelMarkerVisitor(Marker* marker):
    MarkerVisitorBase<true>(marker),
    pool_(),
    work_(){
   }
   ~ParallelMarkerVisitor() override = default;

   bool Visit(RawObject* val) override{
     DLOG(INFO) << "visiting " << val->ToString() << ".....";
     work_.Push(val->GetAddress());
     return true;
   }

   template<typename T>
   void MarkLiveObjects(){
     DLOG(INFO) << "performing ParallelMark....";
     T::VisitRoots(this);
     for(auto idx = 0; idx < TaskPool::kDefaultNumberOfWorkers; idx++)
       pool_.Submit(new ParallelMarkTask(marker_, &work_));
     while(!work_.empty());// spin
   }
 };

 void Marker::MarkLiveObjects(){
   DLOG(INFO) << "marking live objects.....";
#ifdef PSDN_DEBUG
   auto start_ts = Clock::now();
#endif//PSDN_DEBUG

#ifdef PSDN_MTA
   ParallelMarkerVisitor visitor(this);
#else
   SerialMarkerVisitor visitor(this);
#endif//PSDN_MTA
   visitor.MarkLiveObjects<Allocator>();

#ifdef PSDN_DEBUG
   auto finish_ts = Clock::now();
   DLOG(INFO) << "live object marking has finished in " << (finish_ts - start_ts) << ".";
   DLOG(INFO) << "marked " << marked_ << " objects.";
#endif//PSDN_DEBUG
 }
}