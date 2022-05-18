#ifndef POSEIDON_COMPACTOR_H
#define POSEIDON_COMPACTOR_H

#include "poseidon/platform.h"

namespace poseidon{
 class Compactor{
   friend class SerialCompactor;
   friend class ParallelCompactorTask;
  private:
   static void SetCompacting(bool active=true);

   static inline void
   ClearCompacting(){
     return SetCompacting(false);
   }
  public:
   Compactor() = delete;
   Compactor(const Compactor& rhs) = delete;
   ~Compactor() = delete;

   static void Compact();
   static bool IsCompacting();
   static void SerialCompact();
   static void ParallelCompact();

   Compactor& operator=(const Compactor& rhs) = delete;
 };
}

#endif //POSEIDON_COMPACTOR_H