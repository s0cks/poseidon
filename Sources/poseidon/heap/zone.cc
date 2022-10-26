#include "poseidon/heap/zone.h"

namespace poseidon {
 void Zone::Clear(){
   NOT_IMPLEMENTED(ERROR); //TODO: implement
 }

 bool Zone::VisitPages(poseidon::PageVisitor* vis){
   pages_.VisitPages(vis); //TODO: cleanup
   return true;
 }

 bool Zone::VisitMarkedPages(poseidon::PageVisitor* vis){
   pages_.VisitMarkedPages(vis); //TODO: cleanup
   return true;
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