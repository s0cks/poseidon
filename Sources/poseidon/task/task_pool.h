#ifndef POSEIDON_TASK_POOL_H
#define POSEIDON_TASK_POOL_H

#include <random>
#include <ostream>
#include <glog/logging.h>

#include "poseidon/wsq.h"
#include "poseidon/common.h"
#include "poseidon/task/task.h"
#include "poseidon/task/task_worker.h"
#include "poseidon/platform/os_thread.h"

namespace poseidon{
 class TaskPool{
  public:
   typedef WorkStealingQueue<Task*> TaskQueue;
   typedef int32_t QueueSize;

   static constexpr const QueueSize kDefaultMaxQueueSize = 1024;
   static constexpr const size_t kDefaultNumberOfWorkers = 2;
  private:
   WorkerPool wpool_;
  public:
   explicit TaskPool(int64_t num_workers = kDefaultNumberOfWorkers):
    wpool_(num_workers){
   }
   ~TaskPool() = default;

   void Submit(Task* task){
     wpool_.Submit(task);
   }

   template<class T, typename... Args>
   void SubmitToAll(Args... args){
     for(auto widx = 0; widx < wpool_.size(); widx++)
       wpool_.Submit(new T(args...));
   }

   void StartAll(){
     return wpool_.StartAll();
   }

   void ShutdownAll(){
     return wpool_.ShutdownAll();
   }
 };
}

#endif //POSEIDON_TASK_POOL_H