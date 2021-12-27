#ifndef POSEIDON_LIVE_OBJECT_MARKER_H
#define POSEIDON_LIVE_OBJECT_MARKER_H

#include <deque>
#include <glog/logging.h>
#include "raw_object.h"

namespace poseidon{
 class LiveObjectMarker : public RawObjectPointerVisitor{
  protected:
   std::deque<uword>& work_;
   uint64_t marked_;
  public:
   explicit LiveObjectMarker(std::deque<uword>& work):
    RawObjectPointerVisitor(),
    work_(work),
    marked_(0){
   }
   ~LiveObjectMarker() override = default;

   uint64_t GetMarked() const{
     return marked_;
   }

   bool Visit(RawObject* obj) override{
     if(obj->IsMarked())
       return true;
     marked_++;
     obj->SetMarkedBit();
     DLOG(INFO) << "marked @" << obj << " " << obj->ToString();
     return true;
   }

   void MarkLiveObjects(){
     Allocator::VisitLocals(this);
     while(!work_.empty()){
       auto ptr = (RawObject*) work_.front();
       if(ptr && !ptr->IsRemembered()){
         Visit(ptr);
         ptr->SetRememberedBit();
       }
       work_.pop_front();
     }
   }
 };
}

#endif //POSEIDON_LIVE_OBJECT_MARKER_H