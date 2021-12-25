#ifndef POSEIDON_FINALIZER_H
#define POSEIDON_FINALIZER_H

#include <glog/logging.h>
#include "object.h"
#include "raw_object.h"

namespace poseidon{
 class Finalizer : public RawObjectPointerVisitor{
  private:
   uint64_t finalized_;

   inline void
   FinalizeObject(RawObject* raw){
     finalized_++;
     Allocator::FinalizeObject(raw);
   }
  public:
   Finalizer():
    RawObjectPointerVisitor(),
    finalized_(0){
   }
   ~Finalizer() override = default;

   uint64_t GetNumberOfObjectsFinalized() const{
     return finalized_;
   }

   bool Visit(RawObject* raw) override{
     if(!raw->IsMarked() && !raw->IsForwarding()){
       DVLOG(2) << "finalizing @" << raw << " " << raw->ToString();
       FinalizeObject(raw);
     }
     return true;
   }
 };
}

#endif //POSEIDON_FINALIZER_H
