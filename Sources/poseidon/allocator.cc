#include "poseidon/allocator.h"
#include "poseidon/utils.h"
#include "poseidon/local.h"
#include "poseidon/object.h"
#include "poseidon/poseidon.h"

namespace poseidon{
 Heap* Allocator::heap_ = nullptr;

 LocalGroup* Allocator::locals_ = nullptr;
 uint64_t Allocator::num_locals_ = 0;
 uint64_t Allocator::num_allocated_ = 0;

 void Allocator::Initialize(){
   locals_ = new LocalGroup();
   heap_ = new Heap();
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

 void Allocator::VisitLocals(const std::function<bool(RawObject*)>& vis){
   LocalGroup::Iterator iter(locals_, true);
   while(iter.HasNext()){
     if(!vis(iter.Next()))
       return;
   }
 }
}