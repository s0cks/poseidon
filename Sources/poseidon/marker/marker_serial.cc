#include "poseidon/marker/marker_serial.h"

namespace poseidon {
 bool SerialMarker::Visit(RawObject* ptr){
   return Mark(ptr);
 }

 bool SerialMarker::MarkAllRoots(){
   NOT_IMPLEMENTED(ERROR); //TODO: implement
   return false;
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