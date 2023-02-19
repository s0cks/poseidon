#include "poseidon/runtime.h"
#include "poseidon/task/task_pool.h"
#include "poseidon/local/local_page.h"
#include "poseidon/marker/marker_parallel.h"

namespace poseidon {
 class ParallelMarkerTask : public Task {
   friend class ParallelMarker;
  private:
   class ParallelMarkerTaskVisitor : public RawObjectVisitor {
    protected:
     WorkStealingQueue<uword>& queue_;
    public:
     explicit ParallelMarkerTaskVisitor(WorkStealingQueue<uword>& queue):
      queue_(queue) {
     }
     ~ParallelMarkerTaskVisitor() override = default;

     bool Visit(Pointer* ptr) override {
       queue_.Push(ptr->GetStartingAddress());
       return true;
     }
   };
  private:
   ParallelMarker* marker_;
   QueueId queue_id_;
   WorkStealingQueue<uword> queue_;

   inline QueueId queue_id() const {
     return queue_id_;
   }

   inline WorkStealingQueue<uword>& queue() {
     return queue_;
   }

   inline bool HasWork() const {
     return !queue_.IsEmpty();
   }

   inline ParallelMarker* marker() const {
     return marker_;
   }

   inline uword Pop() {
     return queue_.Pop();
   }

   inline uword Steal() {
     return queue_.Steal();
   }
  public:
   explicit ParallelMarkerTask(ParallelMarker* marker, const QueueId queue_id):
    Task(),
    marker_(marker),
    queue_(),
    queue_id_(queue_id) {
   }
   ~ParallelMarkerTask() override = default;

   const char* name() const override {
     return "ParallelMarkTask";
   }

   uword GetNextAddress() {
     auto next = Pop();
     if(next != UNALLOCATED)
       return next;
     //TODO: check queue
     return marker()->StealNextAddress();
   }

   bool Mark(Pointer* ptr) {
     if(ptr->IsMarked())
       return true;
     ptr->SetMarked();
     ParallelMarkerTaskVisitor vis(queue_);
     LOG_IF(FATAL, ptr->VisitPointers(&vis) != ptr->GetPointerSize()) << "failed to visit pointers in " << (*ptr);
     DLOG(INFO) << "marked: " << (*ptr);
     return true;
   }

   void Run() override {
     word idx = 3;
     uword next = UNALLOCATED;
     do {
       while(HasWork() && (next = Pop()) != UNALLOCATED)
         Mark((Pointer*)next);

       while((next = marker()->StealNextAddress()) != UNALLOCATED)
         Mark((Pointer*)next);
     } while(idx-- >= 0);
   }

   std::string ToString() const override {
     std::stringstream ss;
     ss << "ParallelMarkerTask(queue=" << static_cast<word>(queue_id()) << ")";
     return ss.str();
   }
 };

 ParallelMarker::ParallelMarker(Marker* marker):
   MarkerVisitor<true>(marker),
   queue_(kDefaultQueueSize),
   tasks_(new ParallelMarkerTask*[flags::GetNumberOfWorkers()]),
   num_tasks_(flags::GetNumberOfWorkers()),
   engine_(),
   distribution_(0, static_cast<int>(flags::GetNumberOfWorkers())) {
   DLOG(INFO) << "creating ParallelMarker with " << flags::GetNumberOfWorkers() << " workers";
   for(auto idx = 0; idx < num_tasks_; idx++) {
     tasks_[idx] = new ParallelMarkerTask(this, static_cast<QueueId>(idx + 1));
     DLOG(INFO) << "created " << tasks_[idx]->ToString();
   }
 }

 uword ParallelMarker::StealNextAddress() {
   auto queue = GetRandomQueueIndex();
   if(queue == 0)
     return queue_.Steal();
   else if((queue - 1) > num_tasks_)
     return UNALLOCATED;
   auto task = tasks_[queue - 1];
   if(task == nullptr){
     return UNALLOCATED;
   }
   DLOG(INFO) << GetCurrentThreadName() << " stealing from queue #" << (queue - 1) << "/" << num_tasks_;
   return task->Steal();
 }

 void ParallelMarker::MarkAllRoots() {
   SetMarkingRoots();
   DTIMED_SECTION("ParallelMarker::MarkAllRoots", {
     LOG_IF(FATAL, !LocalPageExistsForCurrentThread()) << "no local page exists for current thread";

     auto page = GetLocalPageForCurrentThread();
     ((AllocationSection*)page)->VisitPointers([&](Pointer* ptr) {
       DLOG(INFO) << "visiting: " << (*ptr);
       if(!ptr->IsMarked())
         queue_.Push(ptr->GetStartingAddress());
       return true;
     });

     for(auto idx = 0; idx < num_tasks_; idx++) {
       Runtime::GetTaskPool()->Submit(tasks_[idx]);
     }

     do {
       for(auto idx = 0; idx < num_tasks_; idx++)
         while(!tasks_[idx]->IsFinished()); // spin
     } while(!queue().IsEmpty()); // spin
   });
   SetIdle();
 }
}