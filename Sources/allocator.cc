#include <glog/logging.h>

#include "allocator.h"
#include "utils.h"
#include "local.h"

namespace poseidon{

  LocalGroup* Allocator::locals_ = nullptr;
  MemoryRegion Allocator::region_ = MemoryRegion(GetHeapSize());
  Heap* Allocator::eden_ = nullptr;
  Heap* Allocator::tenured_ = nullptr;
  Heap* Allocator::large_object_ = nullptr;

  void Allocator::Initialize(){
    DLOG(INFO) << "heap size: " << HumanReadableSize(GetHeapSize());
    DLOG(INFO) << "eden space @" << GetEdenSpaceOffset() << " (" << HumanReadableSize(GetEdenSpaceSize()) << ")";
    DLOG(INFO) << "tenured space @" << GetTenuredSpaceOffset() << " (" << HumanReadableSize(GetTenuredSpaceSize()) << ")";
    DLOG(INFO) << "large object space @" << GetLargeObjectSpaceOffset() << " (" << HumanReadableSize(GetLargeObjectSpaceSize()) << ")";
    Allocator::eden_ = new Heap(Space::kEdenSpace, region_.SubRegion(GetEdenSpaceOffset(), GetEdenSpaceSize()));
    Allocator::tenured_ = new Heap(Space::kTenuredSpace, region_.SubRegion(GetTenuredSpaceOffset(), GetTenuredSpaceSize()));
    Allocator::large_object_ = new Heap(Space::kLargeObjectSpace, region_.SubRegion(GetLargeObjectSpaceOffset(), GetLargeObjectSpaceSize()));
    Allocator::locals_ = new LocalGroup();
  }

  RawObject** Allocator::NewLocalSlot(){
    auto group = locals_;
    if(group->GetNumberOfLocals() + 1 >= LocalGroup::kMaxLocalsPerGroup)
      group = new LocalGroup(locals_);
    return &group->locals_[group->num_locals_++];
  }

  void Allocator::VisitLocals(RawObjectPointerPointerVisitor* vis){
    LocalGroup::Iterator iter(locals_, true);
    while(iter.HasNext()){
      vis->Visit(iter.NextPointer());
    }
  }

  void Allocator::VisitLocals(RawObjectPointerVisitor* vis){
    LocalGroup::Iterator iter(locals_, true);
    while(iter.HasNext()){
      vis->Visit(iter.Next());
    }
  }

  void Allocator::VisitLocals(ObjectPointerVisitor* vis){
    LocalGroup::Iterator iter(locals_, true);
    while(iter.HasNext()){
      if(!vis->Visit(iter.Next()->GetObjectPointer()))
        return;
    }
  }
}