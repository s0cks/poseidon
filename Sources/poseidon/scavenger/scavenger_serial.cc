#include "poseidon/local/local_page.h"
#include "poseidon/scavenger/scavenger_serial.h"

namespace poseidon {
 void SerialScavenger::SwapSpaces() {
   return scavenger()->new_zone()->SwapSpaces();
 }

 bool SerialScavenger::ProcessAll() {
   return ProcessRoots() && ProcessToSpace();
 }

 bool SerialScavenger::ProcessRoots() {
   TIMED_SECTION("ProcessRoots", {
     auto page = LocalPage::GetForCurrentThread();
     for(auto idx = 0; idx < page->GetNumberOfLocals(); idx++) {
       auto ptr = page->GetLocalAt(idx);
       if((*ptr) != nullptr) {
         DLOG(INFO) << "checking: " << (**ptr);
         if((*ptr)->IsNew() && (*ptr)->IsMarked() && !(*ptr)->IsForwarding()){
           (*ptr) = (RawObject*) scavenger()->Process((*ptr));
         }
       }
     }
   });
   return true;
 }

 bool SerialScavenger::ProcessToSpace() {
   NOT_IMPLEMENTED(FATAL); //TODO: implement
   return false;
 }

 bool SerialScavenger::Scavenge() {
   SwapSpaces();
   return ProcessRoots();
 }
}
