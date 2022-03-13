#include <glog/logging.h>
#include "poseidon/task_pool.h"

namespace poseidon{
 void TaskPool::Worker::HandleThread(uword parameter){
   auto worker = (TaskPool::Worker*)parameter;

   DLOG(INFO) << "starting worker #" << worker->worker_id() << "....";
   worker->SetState(State::kStarting);
   //TODO: register queue
   worker->SetState(State::kIdle);
   do{
     auto next = worker->queue_->Steal();
     if(!next)
       continue;
     //TODO: calculate timings?
     if(!next->Execute()){
       DLOG(ERROR) << "failed to execute " << next->name() << ".";
       continue;
     }
   } while(!worker->IsStopping());
   DLOG(INFO) << "worker #" << worker->worker_id() << " is stopping....";
   worker->SetState(State::kStopped);
   pthread_exit(nullptr);//TODO: proper exit
 }
}