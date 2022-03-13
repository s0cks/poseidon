#include "poseidon/allocator.h"
#include "poseidon/utils.h"
#include "poseidon/local.h"
#include "poseidon/poseidon.h"
#include "poseidon/os_thread.h"

namespace poseidon{
 static pthread_key_t kHeapThreadLocal;

 static inline void
 SetCurrentThreadHeap(Heap* heap){
   int err;
   if((err = pthread_setspecific(kHeapThreadLocal, (void*)heap)) != 0){
     LOG(ERROR) << "cannot set current thread heap.";
     return;
   }
   DLOG(INFO) << "set heap to " << (*heap) << " for " << GetCurrentThreadName() << ".";
 }

 static inline Heap*
 GetCurrentThreadHeap(){
   void* ptr = nullptr;
   if((ptr = pthread_getspecific(kHeapThreadLocal)) == nullptr){
     LOG(WARNING) << "cannot get current thread heap.";
     return nullptr;
   }
   return (Heap*)ptr;
 }

 LocalGroup* Allocator::locals_ = nullptr;
 uint64_t Allocator::num_locals_ = 0;
 uint64_t Allocator::num_allocated_ = 0;

 void Allocator::Initialize(){
   locals_ = new LocalGroup();
 }

 void Allocator::InitializeForThread(){//TODO: rename
   int err;
   if((err = pthread_key_create(&kHeapThreadLocal, NULL)) != 0){
     LOG(ERROR) << "failed to create heap thread local: " << strerror(err);
     return;
   }
 }

 RawObject** Allocator::NewLocalSlot(){
   auto group = locals_;
   if(group->GetNumberOfLocals() + 1 >= LocalGroup::kMaxLocalsPerGroup)
     group = new LocalGroup(locals_);
   return &group->locals_[group->num_locals_++];
 }

 void Allocator::VisitLocals(RawObjectPointerVisitor* vis){
   LocalGroup::Iterator iter(locals_, true);
   while(iter.HasNext()){
     vis->Visit(iter.NextPointer());
   }
 }

 void Allocator::VisitLocals(RawObjectVisitor* vis){
   LocalGroup::Iterator iter(locals_, true);
   while(iter.HasNext()){
     if(!vis->Visit(iter.Next()))
       return;
   }
 }

 void Allocator::VisitLocals(const std::function<bool(RawObject**)>& vis){
   LocalGroup::Iterator iter(locals_, true);
   while(iter.HasNext()){
     if(!vis(iter.NextPointer()))
       return;
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