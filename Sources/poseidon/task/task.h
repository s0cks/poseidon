#ifndef PSDN_TASK_H
#define PSDN_TASK_H

#include <glog/logging.h>
#include "poseidon/relaxed_atomic.h"
#include "poseidon/platform/platform.h"

namespace poseidon {
#define FOR_EACH_TASK_STATE(V) \
 V(Unqueued)                   \
 V(Queued)                     \
 V(Running)                    \
 V(Successful)                 \
 V(Failed)                     \
 V(Cancelled)                  \
 V(TimedOut)

 class TaskPool;
 class Task{
   friend class WorkerThread;
   friend class WorkerPool;
  public:
   enum State : word {
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
       default:
         return stream << "[unknown Task::State: " << static_cast<word>(val) << "]";
     }
   }
  protected:
   Task* parent_;
   RelaxedAtomic<State> state_;
   RelaxedAtomic<word> children_;

   explicit Task(Task* parent);

   Task():
    Task(nullptr) {
   }

   inline void SetParent(Task* task){
     parent_ = task;
     if(task != nullptr)
       task->children_ += 1;
   }

   inline void SetState(const State& state) {
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

   virtual void OnFinished() const {
     // do nothing
   }

   virtual const char* name() const = 0;

   bool Finish() {
     children_ -= 1;
     if(HasParent())
       return parent()->Finish();
     OnFinished();
     SetState(Task::kSuccessful);
     return true;
   }

   bool Execute(){
     if(IsFinished())
       return false;

     SetState(State::kRunning);
     Run();
     return Finish();
   }

   virtual std::string ToString() const {
     std::stringstream ss;
     ss << name() << "()";
     return ss.str();
   }
 };
}

#endif //PSDN_TASK_H