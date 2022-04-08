#include <glog/logging.h>

#include "poseidon/flags.h"
#include "poseidon/runtime.h"

namespace poseidon{
 static TaskPool* task_pool = nullptr;

 void Runtime::Initialize(){
   DLOG(INFO) << "initializing runtime....";
   task_pool = new TaskPool(GetNumberOfWorkers());
   task_pool->StartAll();
 }

 TaskPool* Runtime::GetTaskPool(){
   return task_pool;
 }
}