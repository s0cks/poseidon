#include "poseidon/local/local_page.h"
#include "poseidon/platform/os_thread.h"
#include "poseidon/marker/marker_serial.h"

namespace poseidon {
 bool SerialMarker::Visit(Pointer* ptr){
   return Mark(ptr);
 }

 bool SerialMarker::MarkAllRoots() { //TODO: iterate pages for roots
   if(!LocalPageExistsForCurrentThread()) {
     DLOG(ERROR) << "no local page exists for current thread `" << GetCurrentThreadName() << "`";
     return false;
   }

   auto page = GetLocalPageForCurrentThread();
   return page->VisitPointers(this);
 }

 bool SerialMarker::MarkAllNewRoots(){
   NOT_IMPLEMENTED(ERROR); //TODO: implement
   return false;
 }

 bool SerialMarker::MarkAllOldRoots(){
   NOT_IMPLEMENTED(ERROR); //TODO: implement
   return false;
 }
}