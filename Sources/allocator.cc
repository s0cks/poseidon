#include <glog/logging.h>

#include "allocator.h"
#include "utils.h"
#include "local.h"
#include "object.h"

namespace poseidon{
 MemoryRegion* Allocator::region_ = nullptr;
 Heap* Allocator::new_ = nullptr;
 Heap* Allocator::old_ = nullptr;

 LocalGroup* Allocator::locals_ = nullptr;
 uint64_t Allocator::num_locals_ = 0;
 uint64_t Allocator::num_allocated_ = 0;

 static const uint64_t kHeapSize = FLAGS_heap_size;
 static const uint64_t kNewSpaceOffset = 0;
 static const uint64_t kNewSpaceSize = kHeapSize / 2;
 static const uint64_t kOldSpaceOffset = kNewSpaceOffset+kNewSpaceSize;
 static const uint64_t kOldSpaceSize = kHeapSize / 2;

 void Allocator::Initialize(){
   locals_ = new LocalGroup();
   region_ = new MemoryRegion(FLAGS_heap_size);
   new_ = new Heap(Space::kNew, region_->SubRegion(kNewSpaceOffset, kNewSpaceSize));
   old_ = new Heap(Space::kOld, region_->SubRegion(kOldSpaceOffset, kOldSpaceSize));
 }

 void Allocator::FinalizeObject(RawObject* raw){
   num_allocated_--;
   raw->GetObjectPointer()->Finalize();
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

 void Allocator::VisitLocals(ObjectPointerVisitor* vis){
   LocalGroup::Iterator iter(locals_, true);
   while(iter.HasNext()){
     if(!vis->Visit(iter.Next()->GetObjectPointer()))
       return;
   }
 }
}