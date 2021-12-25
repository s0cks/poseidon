#ifndef POSEIDON_LIVE_OBJECT_FORWARDER_H
#define POSEIDON_LIVE_OBJECT_FORWARDER_H

#include <glog/logging.h>
#include "object.h"
#include "raw_object.h"

namespace poseidon{
 class LiveObjectForwarder : public RawObjectPointerPointerVisitor{
  public:
   LiveObjectForwarder():
    RawObjectPointerPointerVisitor(){
   }
   ~LiveObjectForwarder() override = default;

   bool Visit(RawObject** ptr) override{
     auto current = *ptr;
     auto next = (RawObject*)current->GetForwardingPointer();
     if(current->IsForwarding()){
       DLOG(INFO) << "forwarding @" << (current) << " " << current->ToString();
       DLOG(INFO) << "to: @" << (next)<< " " << next->ToString();
       next->GetObjectPointer()->SetRawPointer(next->GetAddress());
       (*ptr) = (RawObject*)next;
     }
     return true;
   }
 };
}

#endif //POSEIDON_LIVE_OBJECT_FORWARDER_H
