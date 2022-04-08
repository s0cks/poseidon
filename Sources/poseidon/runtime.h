#ifndef POSEIDON_RUNTIME_H
#define POSEIDON_RUNTIME_H

#include "poseidon/task_pool.h"

namespace poseidon{
 class Runtime{
  public:
   Runtime() = delete;
   Runtime(const Runtime& rhs) = delete;
   ~Runtime() = delete;

   static void Initialize();
   static TaskPool* GetTaskPool();

   Runtime& operator=(const Runtime& rhs) = delete;
 };
}

#endif//POSEIDON_RUNTIME_H