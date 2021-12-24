#ifndef POSEIDON_LIVE_OBJECT_PROMOTER_H
#define POSEIDON_LIVE_OBJECT_PROMOTER_H

#include <glog/logging.h>
#include "live_object_visitor.h"

namespace poseidon{
 class LiveObjectPromoter : public LiveObjectVisitor{
  private:
   Heap* target_;
  public:
   explicit LiveObjectPromoter(Heap* target):
    LiveObjectVisitor(),
    target_(target){
   }
   ~LiveObjectPromoter() override = default;

   Heap* GetTarget() const{
     return target_;
   }

   bool Visit(RawObject* obj) override{
     if(obj->IsReadyForPromotion()){
       auto to_ptr = GetTarget()->AllocateRawObject(obj->GetPointerSize());//TODO: make a better copy
       DLOG(INFO) << "promoting " << obj->GetPointer() << " to " << to_ptr->GetPointer();
       Forward(obj, to_ptr);
       CopyObject(obj, to_ptr);
     }
     return true;
   }
 };
}

#endif //POSEIDON_LIVE_OBJECT_PROMOTER_H