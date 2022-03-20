#include <glog/logging.h>
#include "poseidon/task_pool.h"

namespace poseidon{
 void TaskPool::Worker::HandleThread(uword parameter){
   auto worker = (TaskPool::Worker*)parameter;

   DVLOG(1) << "starting worker #" << worker->worker_id() << "....";
   worker->SetState(State::kStarting);
   // do something?
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
   } while(worker->IsRunning());
   DVLOG(1) << "worker #" << worker->worker_id() << " is stopping....";
   worker->SetState(State::kStopped);
   pthread_exit((void*)"Hello World");//TODO: exit properly
 }
}