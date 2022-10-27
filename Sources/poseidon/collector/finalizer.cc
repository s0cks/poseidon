#include "poseidon/collector/finalizer.h"

namespace poseidon{
 static AtomicPointerCounter finalized_;

 void Finalizer::Reset(){
   finalized_ = 0;
 }

 void Finalizer::Finalize(RawObject* ptr){
   DLOG(INFO) << "finalizing " << (*ptr) << ".";
   memset(ptr->GetPointer(), 0, ptr->GetPointerSize());

   ptr->ClearMarkedBit();
   ptr->SetPointerSize(0);

   finalized_ += ptr;
 }

 AtomicPointerCounter& Finalizer::finalized(){
   return finalized_;
 }
}