#ifndef POSEIDON_FINALIZER_H
#define POSEIDON_FINALIZER_H

#include <glog/logging.h>

#include "poseidon/raw_object.h"

namespace poseidon{
 class Finalizer{
  private:
   uint64_t num_finalized_;
   uint64_t bytes_finalized_;
  public:
   Finalizer():
    num_finalized_(0),
    bytes_finalized_(0){
   }
   ~Finalizer() = default;

   uint64_t GetNumberOfObjectsFinalized() const{
     return num_finalized_;
   }

   uint64_t GetNumberOfBytesFinalized() const{
     return bytes_finalized_;
   }

   void Finalize(RawObject* val){
     DLOG(INFO) << "finalizing " << val->ToString() << ".";
     memset(val->GetPointer(), 0, val->GetPointerSize());

     val->ClearMarkedBit();
     val->SetPointerSize(0);

     num_finalized_ += 1;
     bytes_finalized_ += val->GetTotalSize();
   }
 };
}

#endif //POSEIDON_FINALIZER_H
