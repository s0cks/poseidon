#ifndef POSEIDON_LIVE_OBJECT_MARKER_H
#define POSEIDON_LIVE_OBJECT_MARKER_H

#include <deque>
#include <glog/logging.h>
#include "raw_object.h"

namespace poseidon{
 class LiveObjectMarker : public RawObjectPointerVisitor{
  protected:
   std::deque<uword>& work_;

   static inline void
   MarkObject(RawObject* obj){
     obj->SetMarkedBit();
   }
  public:
   explicit LiveObjectMarker(std::deque<uword>& work):
    RawObjectPointerVisitor(),
    work_(work){
   }
   ~LiveObjectMarker() override = default;

   bool Visit(RawObject* obj) override{
     if(obj->IsMarked())
       return true;
     MarkObject(obj); //TODO: add references to work queue
     return true;
   }
 };
}

#endif //POSEIDON_LIVE_OBJECT_MARKER_H
