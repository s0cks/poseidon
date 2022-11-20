#include "poseidon/local/local_page.h"
#include "poseidon/marker/marker_serial.h"

namespace poseidon {
 bool SerialMarker::Visit(RawObject* ptr){
   return Mark(ptr);
 }

 bool SerialMarker::MarkAllRoots() {
   auto page = LocalPage::GetForCurrentThread();
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