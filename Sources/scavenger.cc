#include <deque>
#include <glog/logging.h>

#include "scavenger.h"
#include "allocator.h"

#include "live_object_marker.h"
#include "live_object_promoter.h"
#include "live_object_scavenger.h"
#include "live_object_forwarder.h"

namespace poseidon{
  void Scavenger::MarkLiveObjects(){
    std::deque<uword> work;

    DLOG(INFO) << "marking root objects....";
    LiveObjectMarker marker(work, Color::kBlack);
    Allocator::VisitLocals(&marker);

    DLOG(INFO) << "marking live objects....";
    while(!work.empty()){
      auto ptr = (RawObject*)work.front();
      if(ptr && !ptr->IsRemembered()){
        marker.Visit(ptr);
        ptr->SetRemembered();
      }
      work.pop_front();
    }
  }

  void Scavenger::PromoteLiveObjects(){
    DLOG(INFO) << "promoting live objects to tenured space....";
    LiveObjectPromoter promoter(Allocator::GetTenuredHeap());
    Allocator::GetEdenHeap()->VisitMarkedRawObjectPointers(&promoter);
  }

  void Scavenger::ScavengeLiveObjects(Heap* heap){
    DLOG(INFO) << "scavenging live objects from " << heap->GetSpace() << "....";
    LiveObjectScavenger scavenger(heap);
    heap->GetFromSpace().VisitMarkedRawObjectPointers(&scavenger);
  }

  void Scavenger::UpdateForwarding(){//TODO: make portable between heaps
    DLOG(INFO) << "updating live object pointers....";
    LiveObjectForwarder forwarder;
    Allocator::VisitLocals(&forwarder);//TODO: visit classes and other runtime values
  }

  void Scavenger::ScavengeFromEdenHeap(){
    DLOG(INFO) << "scavenging from eden heap....";

  }

  void Scavenger::ScavengeFromTenuredHeap(){
    DLOG(INFO) << "scavenging memory from tenured heap....";
    ScavengeLiveObjects(Allocator::GetTenuredHeap());
    Allocator::GetTenuredHeap()->SwapSpaces();//TODO: use sweeper
  }

  void Scavenger::ScavengeFromLargeObjectSpace(){
    DLOG(INFO) << "scavenging memory from eden heap....";
    ScavengeLiveObjects(Allocator::GetEdenHeap());
    UpdateForwarding();
    Allocator::GetEdenHeap()->SwapSpaces();
    Allocator::GetEdenHeap()->GetToSpace().Clear();//TODO: use sweeper
  }

  void Scavenger::Scavenge(){
    DLOG(INFO) << "scavenging memory from heap....";
    MarkLiveObjects();//TODO: make portable between heaps.

    // scavenge from the tenured space
    ScavengeLiveObjects(Allocator::GetTenuredHeap());//TODO: move to after scavenging from eden heap
    Allocator::GetTenuredHeap()->SwapSpaces();

    // scavenge from the eden space
    PromoteLiveObjects();
    ScavengeLiveObjects(Allocator::GetEdenHeap());
    UpdateForwarding();

    DLOG(INFO) << "sweeping heaps....";//TODO: use sweepers
    // swap from & to spaces for compaction
    Allocator::GetEdenHeap()->SwapSpaces();
    Allocator::GetEdenHeap()->GetToSpace().Clear();

    //TODO: clean large object space
  }
}