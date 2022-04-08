#include <glog/logging.h>

#include "poseidon/flags.h"
#include "poseidon/runtime.h"

namespace poseidon{
 static TaskPool task_pool(GetNumberOfWorkers());

 void Runtime::Initialize(){
   DLOG(INFO) << "initializing runtime....";

   DLOG(INFO) << "starting task pool....";
   task_pool.StartAll();
 }

 TaskPool* Runtime::GetTaskPool(){
   return &task_pool;
 }
}