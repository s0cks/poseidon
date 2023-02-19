#ifndef PSDN_TASK_WORKER_H
#define PSDN_TASK_WORKER_H

#include <random>
#include <glog/logging.h>

#include "poseidon/wsq.h"
#include "poseidon/task/task.h"
#include "poseidon/platform/platform.h"
#include "poseidon/platform/os_thread.h"

namespace poseidon {
#define FOR_EACH_TASK_POOL_WORKER_STATE(V) \
   V(Starting)                             \
   V(Idle)                                 \
   V(Executing)                            \
   V(Stopping)                             \
   V(Stopped)

 typedef int8_t WorkerId;

 class WorkerPool;
 class WorkerThread : public OSThread {
   friend class WorkerPool;
  public:
   enum State : int8_t {
#define DEFINE_STATE(Name) k##Name,
     FOR_EACH_TASK_POOL_WORKER_STATE(DEFINE_STATE)
#undef DEFINE_STATE
   };

   friend std::ostream& operator<<(std::ostream& stream, const State& val){
     switch(val){
#define DEFINE_TOSTRING(Name) \
         case State::k##Name: return stream << #Name;
       FOR_EACH_TASK_POOL_WORKER_STATE(DEFINE_TOSTRING)
#undef DEFINE_TOSTRING
       default:
         return stream << "[unknown Worker::State: " << static_cast<word>(val) << "]";
     }
   }
  protected:
   WorkerPool* pool_;
   WorkerId worker_;
   RelaxedAtomic<State> state_;
   WorkStealingQueue<Task*> queue_;

   inline void SetState(const State& state){
     state_ = state;
   }

   inline bool HasWork() const{
     return !queue_.IsEmpty();
   }

   inline Task* Pop() {
     return queue_.Pop();
   }

   inline Task* Steal() {
     return queue_.Steal();
   }

   static inline std::string
   GetWorkerThreadName(const WorkerId worker) {
     std::stringstream ss;
     ss << "worker-" << static_cast<int>(worker);
     return ss.str();
   }

   Task* NextTask();
   void Run() override;
  public:
   explicit WorkerThread(WorkerPool* pool,
                   const WorkerId worker):
                   OSThread(GetWorkerThreadName(worker)),
       pool_(pool),
       worker_(worker),
       state_(State::kStopped),
       queue_() { }
   WorkerThread(const WorkerThread& rhs) = delete;
   ~WorkerThread() = default;

   WorkerPool* pool() const {
     return pool_;
   }

   WorkerId worker_id() const{
     return worker_;
   }

   State state() const{
     return (State)state_;
   }

#define DEFINE_STATE_CHECK(Name) \
     bool Is##Name() const{ return state() == State::k##Name; }
   FOR_EACH_TASK_POOL_WORKER_STATE(DEFINE_STATE_CHECK)
#undef DEFINE_STATE_CHECK

   inline bool IsRunning() const{
     return IsIdle() || IsExecuting();
   }

   WorkerThread& operator=(const WorkerThread& rhs) = delete;
 };

 class WorkerPool{
   friend class WorkerThread;
  protected:
   WorkStealingQueue<Task*> queue_;

   WorkerThread** workers_;
   size_t num_workers_;

   std::default_random_engine engine_;
   std::uniform_int_distribution<WorkerId> distribution_;

   inline WorkerId
   GetRandomWorkerIndex() {
     return distribution_(engine_);
   }

   inline Task* StealRandomTask() {
     auto idx = distribution_(engine_);
     if(idx == 0)
       return queue_.Steal();
     if((idx - 1) >= num_workers_) {
       return nullptr;
     }
     auto worker = workers_[idx - 1];
     if(worker == nullptr)
       return nullptr;
     return worker->Steal();
   }
  public:
   explicit WorkerPool(size_t num_workers):
     queue_(),
     workers_(new WorkerThread*[num_workers]),
     num_workers_(num_workers),
     engine_(Clock::now().time_since_epoch().count()),
     distribution_(0, static_cast<WorkerId>(num_workers)) {
     for(WorkerId widx = 0; widx < static_cast<WorkerId>(num_workers); widx++){
       workers_[widx] = new WorkerThread(this, widx);
     }
   }
   ~WorkerPool(){
     ShutdownAll();
     delete[] workers_;
   }

   size_t size() const{
     return num_workers_;
   }

   WorkerThread** workers() const{
     return workers_;
   }

   WorkerThread* workers(WorkerId id) const{
     if(id < 0 || id > size()){
       DLOG(WARNING) << "cannot get worker #" << id;
       return nullptr;
     }
     DLOG(INFO) << "getting worker #" << id;
     return workers_[id];
   }

   WorkerThread** begin() const{
     return &workers_[0];
   }

   WorkerThread** end() const{
     return &workers_[num_workers_];
   }

   void Submit(Task* task){
     GCLOG(3) << "submitting " << task->ToString() << "....";
     task->SetState(Task::kQueued);
     queue_.Push(task);
   }

   void StartAll() const{
     GCLOG(3) << "starting " << size() << " workers....";
     for(auto& worker : *this)
       worker->Start();
   }

   void ShutdownAll() const{
     GCLOG(3) << "stopping " << size() << " workers....";
     for(auto& worker : *this)
       worker->Join();
   }
 };
}

#endif //PSDN_TASK_WORKER_H