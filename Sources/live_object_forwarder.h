#ifndef POSEIDON_LIVE_OBJECT_FORWARDER_H
#define POSEIDON_LIVE_OBJECT_FORWARDER_H

#include <glog/logging.h>
#include "raw_object.h"

namespace poseidon{
 class LiveObjectForwarder : public RawObjectPointerPointerVisitor{
  public:
   LiveObjectForwarder():
    RawObjectPointerPointerVisitor(){
   }
   ~LiveObjectForwarder() override = default;

   bool Visit(RawObject** ptr) override{
     auto val = *ptr;
     if(val->IsForwarding()){
       (*ptr) = (RawObject*)val->GetForwardingPointer();
       (*ptr)->SetForwardingAddress(0);//TODO: is this necessary?
     }
     return true;
   }
 };
}

#endif //POSEIDON_LIVE_OBJECT_FORWARDER_H
