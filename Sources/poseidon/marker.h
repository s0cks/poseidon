#ifndef POSEIDON_MARKER_H
#define POSEIDON_MARKER_H

#include <glog/logging.h>
#include "poseidon/raw_object.h"
#include "poseidon/relaxed_atomic.h"

namespace poseidon{
 class Marker : public RawObjectVisitor{
  private:
   RelaxedAtomic<uint64_t> num_marked_;
  public:
   Marker():
     num_marked_(0){
   }
   ~Marker() override = default;

   uint64_t GetNumberOfObjectsMarked() const{
     return (uint64_t)num_marked_;
   }

   bool Visit(RawObject* raw) override{
     DVLOG(1) << "marking " << raw->ToString() << ".";
     num_marked_ += 1;
     raw->SetMarkedBit();
     return true;
   }
 };
}

#endif//POSEIDON_MARKER_H