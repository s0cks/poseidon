#ifndef POSEIDON_TASK_POOL_H
#define POSEIDON_TASK_POOL_H

#ifdef PSDN_DEBUG
#include <cassert>
#endif//PSDN_DEBUG

#include <random>
#include <ostream>

#include "poseidon/wsq.h"
#include "poseidon/os_thread.h"
#include "poseidon/relaxed_atomic.h"

namespace poseidon{
#define FOR_EACH_TASK_STATE(V) \
 V(Unqueued)                   \
 V(Queued)                     \
 V(Successful)                 \
 V(Failed)                     \
 V(Cancelled)                  \
 V(TimedOut)

 class TaskPool;
 class Task{
  public:
   enum State : intptr_t{
#define DEFINE_STATE(Name) k##Name,
     FOR_EACH_TASK_STATE(DEFINE_STATE)
#undef DEFINE_STATE
   };

   friend std::ostream& operator<<(std::ostream& stream, const State& val){
     switch(val){
#define DEFINE_TOSTRING(Name) \
       case State::k##Name: return stream << #Name;
       FOR_EACH_TASK_STATE(DEFINE_TOSTRING)
#undef DEFINE_TOSTRING
       default: return stream << "Unknown";
     }
   }
  protected:
   Task* parent_;
   RelaxedAtomic<State> state_;
   RelaxedAtomic<int64_t> children_;

   explicit Task(Task* parent):
    parent_(nullptr),
    state_(State::kUnqueued),
    children_(1){
     SetParent(parent);
   }
   Task():
    Task(nullptr){
   }

   inline void SetParent(Task* task){
     parent_ = task;
     if(task != nullptr)
       task->children_ += 1;
   }

   inline void SetState(const State& state){
     state_ = state;
   }
  public:
   virtual ~Task() = default;

   Task* parent() const{
     return parent_;
   }

   bool HasParent() const{
     return parent() != nullptr;
   }

   State state() const{
     return (State)state_;
   }

#define DEFINE_STATE_CHECK(Name) \
   bool Is##Name() const{ return state() == State::k##Name; }
   FOR_EACH_TASK_STATE(DEFINE_STATE_CHECK)
#undef DEFINE_STATE_CHECK

   bool IsFinished() const{
     return (IsSuccessful() || IsCancelled() || IsFailed() || IsTimedOut()) && children_ == 0;
   }

   virtual void Run() = 0;

   virtual void OnFinished() const{
     // do nothing
   }

   virtual const char* name() const = 0;

   bool Finish(){
     children_ -= 1;
     if(HasParent())
       return parent()->Finish();
     OnFinished();
     return true;
   }

   bool Execute(){
     if(IsFinished())
       return false;
     Run();
     return Finish();
   }
 };

 class TaskPool{
  private:
   typedef WorkStealingQueue<Task*> TaskQueue;

#define FOR_EACH_TASK_POOL_WORKER_STATE(V) \
   V(Starting)                             \
   V(Idle)                                 \
   V(Executing)                            \
   V(Stopping)                             \
   V(Stopped)

   typedef int16_t WorkerId;
   typedef int32_t QueueSize;

   static constexpr const QueueSize kDefaultMaxQueueSize = 1024;
   static constexpr const size_t kDefaultNumberOfWorkers = 2;

   class Worker{
    public:
     enum State : intptr_t{
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
         default: return stream << "Unknown";
       }
     }
    private:
     const TaskPool* pool_;
     ThreadId thread_;
     WorkerId worker_;
     RelaxedAtomic<State> state_;
     TaskQueue* queue_;

     inline void SetState(const State& state){
       state_ = state;
     }

     static void HandleThread(uword parameter);
    public:
     Worker(const TaskPool* pool, WorkerId worker, TaskQueue* queue):
       pool_(pool),
       thread_(),
       worker_(worker),
       state_(State::kStopped),
       queue_(queue){
     }
     Worker(const Worker& rhs) = delete;
     ~Worker() = default;

     const TaskPool* pool() const{
       return pool_;
     }

     ThreadId thread_id() const{
       return thread_;
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

     bool Start(){
       if(!IsStopped())
         return false;
       char thread_name[kThreadNameMaxLength];
       snprintf(thread_name, kThreadNameMaxLength, "worker-%d", (int)worker_id());
       return ::poseidon::Start(&thread_, thread_name, &HandleThread, (uword)this);
     }

     bool Shutdown(){
       return Join(thread_);
     }

     Worker& operator=(const Worker& rhs) = delete;
   };

   class WorkerPool{
    private:
     TaskQueue queue_;

     Worker** workers_;
     size_t num_workers_;

     std::default_random_engine engine_;
     std::uniform_int_distribution<WorkerId> distribution_;

     inline WorkerId
     GetRandomWorkerIndex(){
       return distribution_(engine_);
     }
    public:
     WorkerPool(const TaskPool* pool, size_t num_workers, int64_t queue_size = kDefaultMaxQueueSize):
      queue_(queue_size),
      workers_(nullptr),
      num_workers_(num_workers),
      engine_(Clock::now().time_since_epoch().count()),
      distribution_(0, static_cast<WorkerId>(num_workers) - 1){
       if(num_workers > 0){
         workers_ = new Worker*[num_workers];
         for(auto idx = 0; idx < num_workers; idx++){
           workers_[idx] = new Worker(pool, static_cast<WorkerId>(idx), &queue_);
           if(!workers_[idx]->Start())
             LOG(ERROR) << "cannot start worker #" << idx << ".";
         }
       }
     }
     ~WorkerPool(){
       for(auto& worker : *this){
         if(!worker->Shutdown())
           LOG(ERROR) << "cannot shutdown worker #" << worker->worker_id() << ".";
       }
       delete[] workers_;
     }

     size_t size() const{
       return num_workers_;
     }

     Worker** workers() const{
       return workers_;
     }

     Worker* workers(WorkerId id) const{
       if(id < 0 || id > size()){
         DLOG(WARNING) << "cannot get worker #" << id;
         return nullptr;
       }
       DLOG(INFO) << "getting worker #" << id;
       return workers_[id];
     }

     Worker* random(){
       return workers(GetRandomWorkerIndex());
     }

     Worker** begin() const{
       return &workers_[0];
     }

     Worker** end() const{
       return &workers_[num_workers_];
     }

     void Submit(Task* task){
       queue_.Push(task);
     }

     void StartAll() const{
       for(auto& worker : *this){
         if(!worker->Start())
           LOG(ERROR) << "cannot start worker #" << worker->worker_id() << ".";
       }
     }

     void ShutdownAll() const{
       for(auto& worker : *this){
         if(!worker->Shutdown())
           LOG(ERROR) << "cannot shutdown worker #" << worker->worker_id() << ".";
       }
     }
   };
  private:
   WorkerPool wpool_;
  public:
   explicit TaskPool(size_t num_workers = kDefaultNumberOfWorkers):
    wpool_(this, num_workers){
   }
   ~TaskPool() = default;

   void Submit(Task* task){
     wpool_.Submit(task);
   }
 };
}

#endif //POSEIDON_TASK_POOL_H