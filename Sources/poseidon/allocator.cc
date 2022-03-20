#include "poseidon/utils.h"
#include "poseidon/local.h"
#include "poseidon/marker.h"
#include "poseidon/allocator.h"
#include "poseidon/os_thread.h"
#include "poseidon/scavenger.h"

namespace poseidon{
 static pthread_key_t kHeapThreadLocal;

 RootPage* Allocator::roots_ = nullptr;

 void Allocator::Initialize(){
   roots_ = new RootPage();
 }

 void Allocator::InitializeForThread(){//TODO: rename
   int err;
   if((err = pthread_key_create(&kHeapThreadLocal, nullptr)) != 0){
     LOG(ERROR) << "failed to create heap thread local: " << strerror(err);
     return;
   }
 }

 bool Allocator::IsRoot(uword address){
   auto page = roots_;
   while(page != nullptr){
     if(page->Contains(address))
       return true;
   }
   return false;
 }

 void Allocator::VisitRoots(const std::function<bool(uword, uword)>& vis){
   auto page = roots_;
   while(page != nullptr){
     page->VisitPointers(vis);
     page = page->GetNext();
   }
 }

 void Allocator::VisitRoots(RawObjectVisitor* vis){
   auto page = roots_;
   while(page != nullptr){
     page->VisitPointers(vis);
     page = page->GetNext();
   }
 }

 void Allocator::VisitRoots(RawObjectPointerVisitor* vis){
   auto page = roots_;
   while(page != nullptr){
     page->VisitPointers(vis);
     page = page->GetNext();
   }
 }

 void Allocator::VisitMarkedRoots(RawObjectVisitor* vis){
   auto page = roots_;
   while(page != nullptr){
     page->VisitMarkedPointers(vis);
     page = page->GetNext();
   }
 }

 void Allocator::VisitNewRoots(RawObjectVisitor* vis){
   auto page = roots_;
   while(page != nullptr){
     page->VisitNewPointers(vis);
     page = page->GetNext();
   }
 }

 void Allocator::VisitOldRoots(RawObjectVisitor* vis){
   auto page = roots_;
   while(page != nullptr){
     page->VisitOldPointers(vis);
     page = page->GetNext();
   }
 }

 void Allocator::MinorCollection(){
   DLOG(INFO) << "starting minor collection.....";
#ifdef PSDN_DEBUG
   auto start_ts = Clock::now();
#endif//PSDN_DEBUG

   Scavenger::ScavengeFromCurrentThreadHeap();

#ifdef PSDN_DEBUG
   auto finish_ts = Clock::now();
   DLOG(INFO) << "minor collection finished in " << (finish_ts - start_ts) << ".";
#endif//PSDN_DEBUG
 }

 void Allocator::MajorCollection(){

 }
}