#include <deque>
#include <glog/logging.h>
#include "scavenger.h"
#include "allocator.h"
#include "marker.h"
#include "object.h"

namespace poseidon{
  void Scavenger::MarkLiveObjects(){
    std::deque<uword> work;

    DLOG(INFO) << "marking root objects....";
    LiveObjectMarker marker(work, Color::kBlack);
    Allocator::VisitLocals(&marker);

    DLOG(INFO) << "marking live objects....";
    while(!work.empty()){
      auto ptr = (RawObject*)work.front();
      if(ptr && !ptr->IsRemembered())
        marker.Visit(ptr);
      work.pop_front();
    }
  }

  //TODO: split promotion and scavenging
  class LiveObjectPromoter : public RawObjectPointerVisitor{
   private:
    Heap* from_;
    Heap* to_;
   public:
    explicit LiveObjectPromoter(Heap* from, Heap* to):
      RawObjectPointerVisitor(),
      from_(from),
      to_(to){
    }
    ~LiveObjectPromoter() override = default;

    Heap* GetFrom() const{
      return from_;
    }

    Heap* GetTo() const{
      return to_;
    }

    bool Visit(RawObject* obj) override{
      if(obj->IsMarked()){
        if(obj->IsReadyForPromotion()){
          DLOG(INFO) << "promoting " << obj->ToString();
          auto new_ptr = GetTo()->AllocateRawObject(obj->GetPointerSize());//TODO: make a better copy
          obj->SetForwardingAddress(new_ptr->GetAddress());
        } else{
          DLOG(INFO) << "scavenging " << obj->ToString();
          auto new_ptr = GetFrom()->GetToSpace().AllocateRawObject(obj->GetPointerSize());//TODO: make a better copy
          obj->SetForwardingAddress(new_ptr->GetAddress());
        }
      }
      return true;
    }
  };

  //TODO: rename?
  class LiveObjectScavenger : public RawObjectPointerVisitor{
   private:
    Semispace target_;
   public:
    explicit LiveObjectScavenger(Semispace target):
      RawObjectPointerVisitor(),
      target_(target){
    }
    ~LiveObjectScavenger() override = default;

    Semispace& GetTarget(){
      return target_;
    }

    bool Visit(RawObject* obj) override{
      if(obj->IsForwarding()){
        DLOG(INFO) << "forwarding " << obj->ToString();
        auto new_ptr = ((RawObject*)obj->GetForwardingAddress());
        memcpy(new_ptr->GetPointer(), obj->GetPointer(), obj->GetPointerSize());
      }
      return true;
    }
  };

  void Scavenger::PromoteLiveObjects(){
    DLOG(INFO) << "promoting live objects to tenured space....";
    LiveObjectPromoter promoter(Allocator::GetEdenHeap(), Allocator::GetTenuredHeap());
    Allocator::GetEdenHeap()->VisitMarkedRawObjectPointers(&promoter);
  }

  void Scavenger::ScavengeLiveObjects(Heap* heap){
    DLOG(INFO) << "scavenging live objects from " << heap->GetSpace() << "....";
    LiveObjectScavenger scavenger(heap->GetToSpace());
    heap->GetFromSpace().VisitMarkedRawObjectPointers(&scavenger);
  }

  class Sweeper : public RawObjectPointerVisitor{
   public:
    Sweeper() = default;
    ~Sweeper() override = default;

    bool Visit(RawObject* obj) override{
      if(obj->IsForwarding()){
        obj->SetPointerAddress(obj->GetForwardingAddress());
        obj->SetForwardingAddress(0);
      }
      return true;
    }
  };

  class LiveObjectForwarder : public RawObjectPointerPointerVisitor{
   public:
    LiveObjectForwarder() = default;
    ~LiveObjectForwarder() override = default;

    bool Visit(RawObject** ptr) override{
      auto obj = (*ptr);
      if(obj->IsForwarding()){
        DLOG(INFO) << "forwarding " << obj->ToString();
        (*ptr) = (RawObject*)obj->GetForwardingPointer();
      }
      return true;
    }
  };

  void Scavenger::UpdateLocals(){
    LiveObjectForwarder forwarder;
    Allocator::VisitLocals(&forwarder);
  }

  void Scavenger::Scavenge(){
    DLOG(INFO) << "scavenging memory from heap....";
    MarkLiveObjects();

    ScavengeLiveObjects(Allocator::GetTenuredHeap());
    Allocator::GetTenuredHeap()->SwapSpaces();

    PromoteLiveObjects();
    ScavengeLiveObjects(Allocator::GetEdenHeap());
    UpdateLocals();
    Allocator::GetEdenHeap()->SwapSpaces();
    Allocator::GetEdenHeap()->GetToSpace().Clear();
  }
}