#include <glog/logging.h>

#include "poseidon/common.h"
#include "poseidon/task_pool.h"

namespace poseidon{
 void TaskPool::Worker::HandleThread(uword parameter){
   auto worker = (TaskPool::Worker*)parameter;
   DLOG(INFO) << "starting worker #" << worker->worker_id() << "....";
   worker->SetState(State::kStarting);
   // do something?
   worker->SetState(State::kExecuting);
   do{
     do{
       auto next = (Task*)worker->queue_->Steal();
       if (!next)
         continue;

       DTIMED_SECTION(next->name(), {
         if(!next->Execute()){
           DLOG(ERROR) << "failed to execute " << next->name() << ".";
           continue;
         }
       });
     } while(worker->HasWork());
   } while(worker->IsRunning());
   DLOG(INFO) << "worker #" << worker->worker_id() << " is stopping....";
   worker->SetState(State::kStopped);
   pthread_exit((void*)"Hello World");//TODO: exit properly
 }
}