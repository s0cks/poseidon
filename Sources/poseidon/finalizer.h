#ifndef POSEIDON_FINALIZER_H
#define POSEIDON_FINALIZER_H

#include <glog/logging.h>
#include "object.h"
#include "raw_object.h"

namespace poseidon{
 class Finalizer : public RawObjectVisitor{
  private:
   uint64_t finalized_;
  public:
   Finalizer():
    RawObjectVisitor(),
    finalized_(0){
   }
   ~Finalizer() override = default;

   uint64_t GetNumberOfObjectsFinalized() const{
     return finalized_;
   }

   bool Visit(RawObject* raw) override{
     return true;
   }
 };
}

#endif //POSEIDON_FINALIZER_H
