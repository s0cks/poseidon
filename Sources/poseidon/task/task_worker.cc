#include "poseidon/common.h"
#include "poseidon/task/task_worker.h"

namespace poseidon {
 Task* WorkerThread::NextTask() {
   auto next = Pop();
   if(next != nullptr)
     return next;
   return pool()->StealRandomTask();
 }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
#pragma ide diagnostic ignored "UnreachableCode"
 void WorkerThread::Run() {
   SetState(State::kStarting);
   // do something?
   SetState(State::kExecuting);
   while(IsRunning()) {
     auto next = NextTask();
     if(next != nullptr) {
       DTIMED_SECTION(next->ToString(), {
         if(!next->Execute()) {
           LOG(ERROR) << "failed to execute " << next->name() << ".";
           next->SetState(Task::kFailed);
           break;
         }
       });
     }
   }
   SetState(State::kStopped);
   pthread_exit((void*)"Hello World");//TODO: exit properly
 }
#pragma clang diagnostic pop
}