#ifndef POSEIDON_FINALIZER_H
#define POSEIDON_FINALIZER_H

#include <glog/logging.h>

#include "poseidon/raw_object.h"

namespace poseidon{
 class Finalizer{
   friend class Scavenger;
   friend class ParallelScavenger;

   friend class SerialCompactor;
   friend class ParallelCompactorTask;

   friend class SerialSweeper;
   friend class ParallelSweeperTask;
  private:
   static void Reset();
   static void Finalize(RawObject* ptr);
   static AtomicPointerCounter& finalized();
  public:
   Finalizer() = delete;
   Finalizer(const Finalizer& rhs) = delete;
   ~Finalizer() = delete;

   static inline int64_t
   GetNumberOfObjectsFinalized(){
     return (int64_t)finalized().count;
   }

   static inline int64_t
   GetNumberOfBytesFinalized(){
     return (int64_t)finalized().bytes;
   }

   Finalizer& operator=(const Finalizer& rhs) = delete;
 };
}

#endif //POSEIDON_FINALIZER_H
