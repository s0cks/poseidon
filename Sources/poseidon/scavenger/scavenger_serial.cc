#include "poseidon/local.h"
#include "poseidon/scavenger/scavenger_serial.h"

namespace poseidon {
 void SerialScavenger::SwapSpaces() {
   return scavenger()->new_zone()->SwapSpaces();
 }

 bool SerialScavenger::ProcessAll() {
   return ProcessRoots() && ProcessToSpace();
 }

 static inline uword Forward(RawObject* src, uword dst) {
   if(src->IsForwarding())
     return src->GetForwardingAddress();
   src->SetForwardingAddress(dst);
   return src->GetForwardingAddress();
 }

 uword SerialScavenger::Process(RawObject* ptr) {
   DLOG(INFO) << "processing " << (*ptr);
   if(ptr->IsForwarding())
     return ptr->GetForwardingAddress();

   if(ptr->IsNew() && ptr->IsRemembered())
     return Forward(ptr, scavenger()->Promote(ptr));
   return Forward(ptr, scavenger()->Scavenge(ptr));
 }

 bool SerialScavenger::ProcessRoots() {
   TIMED_SECTION("ProcessLocals", {
     auto locals = LocalPage::GetLocalPageForCurrentThread();
     locals->VisitPointers([&](RawObject** ptr){
       auto old_val = (*ptr);
       if(old_val->IsNew() && !old_val->IsForwarding()){
         auto new_val = (RawObject*)Process(old_val);
         (*ptr) = new_val;
       }
       return true;
     });
   });
   return true;
 }

  bool SerialScavenger::NotifyLocals(){
   DTIMED_SECTION("NotifyLocals", {
     auto locals = LocalPage::GetLocalPageForCurrentThread();
     locals->VisitPointers([&](RawObject** ptr){
       auto old_val = (*ptr);
       DLOG(INFO) << "notifying " << (*old_val);
       if(old_val->IsNew() && old_val->IsForwarding()){
         (*ptr) = (RawObject*)old_val->GetForwardingAddress();
       }
       return true;
     });
   });
   return true;
  }

 bool SerialScavenger::ProcessToSpace() {
   NOT_IMPLEMENTED(FATAL); //TODO: implement
   return false;
 }

 bool SerialScavenger::Scavenge() {
   SwapSpaces();

   if(!ProcessRoots()) {
     DLOG(ERROR) << "cannot process roots";
     return false;
   }

   if(!NotifyLocals()) {
     DLOG(ERROR) << "cannot notify locals";
     return false;
   }
   return true;
 }
}
