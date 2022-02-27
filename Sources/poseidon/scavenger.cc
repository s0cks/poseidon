#include <deque>
#include <glog/logging.h>

#include "utils.h"
#include "scavenger.h"
#include "allocator.h"

#include "finalizer.h"

namespace poseidon{
 uword Scavenger::PromoteObject(RawObject* obj){
   auto new_ptr = Allocator::GetHeap()->old_zone().AllocateRawObject(obj->GetPointerSize());
   new_ptr->SetOldBit();
   new_ptr->GetObjectPointer()->set_raw(new_ptr);
   memcpy(new_ptr->GetPointer(), obj->GetPointer(), obj->GetPointerSize());
   DLOG(INFO) << "promoted " << obj->ToString() << " to " << new_ptr->ToString();
   return new_ptr->GetAddress();
 }

 uword Scavenger::ScavengeObject(RawObject* obj){
   auto new_ptr = to_space_.AllocateRawObject(obj->GetPointerSize());
   new_ptr->SetNewBit();
   new_ptr->SetRememberedBit();
   new_ptr->GetObjectPointer()->set_raw(new_ptr);
   memcpy(new_ptr->GetPointer(), obj->GetPointer(), obj->GetPointerSize());
   DLOG(INFO) << "relocated " << obj->ToString() << " to " << new_ptr->ToString();
   return new_ptr->GetAddress();
 }

 uword Scavenger::CopyObject(RawObjectPtr raw){
   if(!raw->IsForwarding()){
     if(raw->IsNew() && !raw->IsMarked() && raw->IsRemembered()){
       MarkObject(raw);
       auto new_address = PromoteObject(raw);
       ForwardObject(raw, new_address);
     } else if(raw->IsNew() && !raw->IsMarked() && !raw->IsRemembered()){
       MarkObject(raw);
       auto new_address = ScavengeObject(raw);
       ForwardObject(raw, new_address);
     }
   }
   return raw->GetForwardingAddress();
 }

 bool Scavenger::Visit(RawObjectPtr* ptr){
   auto old_val = (*ptr);
   if(!old_val->IsMarked() && !old_val->IsRemembered()){
     (*ptr) = (RawObject*)CopyObject(old_val);
   }
   return true;
 }

 bool Scavenger::Visit(RawObjectPtr ptr){
   ptr->VisitPointers(this);
   return true;
 }

 void Scavenger::SwapSpaces(){
   zone_.SwapSpaces();
 }

 void Scavenger::ProcessRoots(){
   DLOG(INFO) << "processing " << Allocator::GetNumberOfLocals() << " roots.";
   Allocator::VisitLocals(this);
 }

 void Scavenger::ProcessToSpace(){
   DLOG(INFO) << "processing to-space " << to_space_;
   to_space_.VisitRawObjects([&](RawObject* val){
     val->VisitPointers(this);
     return true;
   });
 }

 void Scavenger::ProcessCopiedObjects(){
   NOT_IMPLEMENTED(ERROR);//TODO: implement
 }

 void Scavenger::MinorCollection(){
   // only collect from the eden heap
   DLOG(INFO) << "executing minor collection.";
   auto heap = Allocator::GetHeap();
   auto new_zone = heap->new_zone();

#ifdef PSDN_DEBUG
   auto start_ts = Clock::now();
#endif//PSDN_DEBUG

   Scavenger scavenger(new_zone);
   new_zone.SwapSpaces();
   scavenger.ProcessRoots(); // scavenge roots into to_
   scavenger.ProcessToSpace(); // scavenge from_ into to_

#ifdef PSDN_DEBUG
   auto finished_ts = Clock::now();
#endif//PSDN_DEBUG
 }

 void Scavenger::MajorCollection(){

 }
}