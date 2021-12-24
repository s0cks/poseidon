#ifndef POSEIDON_LIVE_OBJECT_MARKER_H
#define POSEIDON_LIVE_OBJECT_MARKER_H

#include <deque>
#include <glog/logging.h>
#include "raw_object.h"

namespace poseidon{
 class LiveObjectMarker : public RawObjectPointerVisitor{
  protected:
   Color color_;
   std::deque<uword>& work_;

   inline void
   MarkObject(RawObject* obj) const{
     obj->SetColor(GetColor());
   }
  public:
   LiveObjectMarker(std::deque<uword>& work, const Color& color):
    RawObjectPointerVisitor(),
    color_(color),
    work_(work){
   }
   explicit LiveObjectMarker(std::deque<uword>& work):
     LiveObjectMarker(work, Color::kBlack){
   }
   ~LiveObjectMarker() override = default;

   Color GetColor() const{
     return color_;
   }

   bool Visit(RawObject* obj) override{
     if(obj->GetColor() != Color::kFree)
       return true;
     MarkObject(obj); //TODO: add references to work queue
     return true;
   }
 };
}

#endif //POSEIDON_LIVE_OBJECT_MARKER_H
