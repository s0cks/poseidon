#include <glog/logging.h>

#include "poseidon/flags.h"
#include "poseidon/runtime.h"

namespace poseidon{
 static TaskPool* task_pool = nullptr;

 void Runtime::Initialize(){
   GCLOG(3) << "initializing Runtime....";
   task_pool = new TaskPool(flags::FLAGS_num_workers);
   task_pool->StartAll();
 }

 TaskPool* Runtime::GetTaskPool(){
   return task_pool;
 }
}