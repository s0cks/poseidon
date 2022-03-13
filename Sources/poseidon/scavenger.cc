#include <deque>
#include <glog/logging.h>

#include "poseidon/utils.h"
#include "poseidon/scavenger.h"
#include "poseidon/allocator.h"
#include "poseidon/finalizer.h"

namespace poseidon{
 uword Scavenger::PromoteObject(RawObject* obj){
   DLOG(INFO) << "promoting " << obj->ToString() << " to new zone.";
   auto new_ptr = Heap::GetCurrentThreadHeap()->old_zone()->AllocateRawObject(obj->GetPointerSize());
   CopyObject(obj, new_ptr);
   new_ptr->SetOldBit();

//   stats_.num_promoted += 1;
//   stats_.bytes_promoted += obj->GetPointerSize();
   return new_ptr->GetAddress();
 }

 uword Scavenger::ScavengeObject(RawObject* obj){
   DLOG(INFO) << "scavenging " << obj->ToString() << " in old zone.";
//   auto new_ptr = zone()->AllocateRawObject(obj->GetPointerSize());
   auto new_ptr = new RawObject();
   CopyObject(obj, new_ptr);
   new_ptr->SetNewBit();

//   stats_.num_scavenged += 1;
//   stats_.bytes_scavenged += obj->GetPointerSize();
   return new_ptr->GetAddress();
 }

 uword Scavenger::ProcessObject(RawObject* raw){
   if(!raw->IsForwarding()){
     if(!raw->IsMarked() && raw->IsRemembered()){
       auto new_address = PromoteObject(raw);
       ForwardObject(raw, new_address);
     } else if(!raw->IsMarked() && !raw->IsRemembered()){
       auto new_address = ScavengeObject(raw);
       ForwardObject(raw, new_address);
     }
   }
   raw->SetRememberedBit();
   return raw->GetForwardingAddress();
 }

 void Scavenger::ProcessRoots(){
   DLOG(INFO) << "processing roots....";
   Allocator::VisitLocals([&](RawObject** val){
     auto old_val = (*val);
     DLOG(INFO) << "visiting " << old_val->ToString();
     if(old_val->IsNew() && !old_val->IsMarked()){
       DLOG(INFO) << "processing root: " << old_val->ToString();
       (*val) = (RawObject*)ProcessObject(old_val);
     }
     (*val)->SetRememberedBit();
     return true;
   });
   DLOG(INFO) << "processed roots.";
 }

 void Scavenger::ProcessToSpace() const{
   DLOG(INFO) << "processing to-space....";
//   GetToSpace().VisitRawObjects([&](RawObject* val){
//     if(val->IsNew() && val->IsMarked() && !val->IsRemembered()){
//       DLOG(INFO) << "processing " << val->ToString();
//     }
//     return true;
//   });
 }

 void Scavenger::SwapSpaces() const{

 }

 void Scavenger::Scavenge(){
   SwapSpaces();
   ProcessRoots();
   ProcessToSpace();

//   zone()->to().Clear();
 }
}