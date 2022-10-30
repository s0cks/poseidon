#include "poseidon/marker/marker_serial.h"

namespace poseidon {
 bool SerialMarker::Visit(RawObject* ptr){
   ptr->SetMarkedBit();
   //TODO: push the pointers that ptr holds to work_
   return ptr->IsMarked();
 }

 bool SerialMarker::MarkAllRoots(){
   auto page = LocalPage::GetLocalPageForCurrentThread();
   while(page != nullptr) {
     LocalPage::LocalPageIterator iter(page);
     while(iter.HasNext()) {
       auto next = iter.Next();
       if(!Visit(next))
         return false;
     }
     page = page->GetNext();
   }
   return true;
 }

 bool SerialMarker::MarkAllNewRoots(){
   auto page = LocalPage::GetLocalPageForCurrentThread();
   while(page != nullptr) {
     LocalPage::LocalPageIterator iter(page);
     while(iter.HasNext()) {
       auto next = iter.Next();
       if(next->IsNew() && !Visit(next))
         return false;
     }
     page = page->GetNext();
   }
   return true;
 }

 bool SerialMarker::MarkAllOldRoots(){
   auto page = LocalPage::GetLocalPageForCurrentThread();
   while(page != nullptr) {
     LocalPage::LocalPageIterator iter(page);
     while(iter.HasNext()) {
       auto next = iter.Next();
       if(next->IsOld() && !Visit(next))
         return false;
     }
     page = page->GetNext();
   }
   return true;
 }
}