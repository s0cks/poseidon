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
   if(!LocalPageExistsForCurrentThread()) {
     LOG(WARNING) << "no LocalPage exists for thread `" << GetCurrentThreadName() << "`";
     return false;
   }

   TIMED_SECTION("ProcessRoots", {
     auto page = GetLocalPageForCurrentThread();
     LOG_IF(FATAL, !page->VisitNewPointers(this)) << "failed to visit new pointers in " << (*page);
   });
   return true;
 }

 bool SerialScavenger::Visit(Pointer* ptr) {
   if(ptr->IsNew() && ptr->IsMarked()) {
     DLOG(INFO) << "visiting " << (*ptr);
   }
   return true;
 }

 bool SerialScavenger::ProcessToSpace() {
   NOT_IMPLEMENTED(FATAL); //TODO: implement
   return false;
 }

 void SerialScavenger::Scavenge() {
   SwapSpaces();
   LOG_IF(FATAL, !ProcessRoots()) << "failed to process roots.";
 }
}
