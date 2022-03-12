#include <glog/logging.h>
#include "poseidon/task_pool.h"

namespace poseidon{
 Task* TaskPool::Worker::GetNextTask(){
   auto next = queue_.Pop();
   if(next)
     return next;
   return nullptr;//TODO: implement
 }

 void TaskPool::Worker::HandleThread(uword parameter){
   auto worker = (TaskPool::Worker*)parameter;

   DLOG(INFO) << "starting worker #" << worker->worker_id() << "....";
   worker->SetState(State::kStarting);
   //TODO: register queue
   worker->SetState(State::kIdle);
   do{
     auto next = worker->GetNextTask();
     if(!next)
       continue;
     //TODO: calculate timings?
     if(!next->Execute())
       continue;
   } while(!worker->IsStopping());
   DLOG(INFO) << "worker #" << worker->worker_id() << " is stopping....";
   worker->SetState(State::kStopped);
   pthread_exit(nullptr);//TODO: proper exit
 }
}