#ifndef POSEIDON_LIVE_OBJECT_SCAVENGER_H
#define POSEIDON_LIVE_OBJECT_SCAVENGER_H

#include <glog/logging.h>
#include "live_object_promoter.h"

namespace poseidon{
 class LiveObjectScavenger : public LiveObjectVisitor{
  private:
   Heap* target_;
  public:
   explicit LiveObjectScavenger(Heap* target):
    LiveObjectVisitor(),
    target_(target){
   }
   ~LiveObjectScavenger() override = default;

   Heap* GetTarget() const{
     return target_;
   }

   bool Visit(RawObject* obj) override{
     auto to_ptr = GetTarget()->GetToSpace().AllocateRawObject(obj->GetPointerSize());//TODO: make a better copy
     DLOG(INFO) << "scavenging " << obj->GetPointer() << to_ptr->GetPointer();
     Forward(obj, to_ptr);
     CopyObject(obj, to_ptr);
     return true;
   }
 };
}

#endif//POSEIDON_LIVE_OBJECT_SCAVENGER_H