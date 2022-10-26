#include "poseidon/heap/zone.h"

namespace poseidon {
 void Zone::Clear(){
   NOT_IMPLEMENTED(ERROR); //TODO: implement
 }

 bool Zone::VisitPointers(poseidon::RawObjectVisitor* vis){
   NOT_IMPLEMENTED(ERROR); //TODO: implement
   return false;
 }

 bool Zone::VisitMarkedPointers(poseidon::RawObjectVisitor* vis){
   NOT_IMPLEMENTED(ERROR); //TODO: implement
   return false;
 }
}