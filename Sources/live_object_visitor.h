#ifndef POSEIDON_LIVE_OBJECT_VISITOR_H
#define POSEIDON_LIVE_OBJECT_VISITOR_H

#include "heap.h"
#include "raw_object.h"

namespace poseidon{
 class LiveObjectVisitor : public RawObjectPointerVisitor{
  protected:
   LiveObjectVisitor() = default;

   static inline void
   Forward(RawObject* from, RawObject* to){
     from->SetForwardingAddress(to->GetAddress());
   }

   static inline void
   CopyObject(RawObject* from, RawObject* to){
     //TODO: size checks and faster/better copy?
     memcpy(to->GetPointer(), from->GetPointer(), from->GetPointerSize());
   }
  public:
   ~LiveObjectVisitor() override = default;
 };
}

#endif //POSEIDON_LIVE_OBJECT_VISITOR_H
