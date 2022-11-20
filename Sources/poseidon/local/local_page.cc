#include "poseidon/raw_object.h"
#include "poseidon/local/local_page.h"
#include "poseidon/platform/os_thread.h"

namespace poseidon {
 bool LocalPage::VisitPointers(RawObjectVisitor* vis){
   LocalPageIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(!vis->Visit(next))
       return false;
   }
   return true;
 }

 bool LocalPage::VisitNewPointers(RawObjectVisitor* vis){
   LocalPageIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(next->IsNew() && !vis->Visit(next))
       return false;
   }
   return true;
 }

 bool LocalPage::VisitOldPointers(RawObjectVisitor* vis){
   LocalPageIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(next->IsOld() && !vis->Visit(next))
       return false;
   }
   return true;
 }

 bool LocalPage::VisitMarkedPointers(RawObjectVisitor* vis){
   LocalPageIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(next->IsMarked() && !vis->Visit(next))
       return false;
   }
   return true;
 }

 uword LocalPage::TryAllocate() {
   if((GetCurrentAddress() + kWordSize) > GetEndingAddress())
     return UNALLOCATED; //TODO: maybe clean this page or do something else?
   auto address = GetCurrentAddress();
   current_ += kWordSize;
   return address;
 }

 static ThreadLocalKey kLocalPageThreadLocalKey;

 void LocalPage::Initialize() {
   LOG_IF(FATAL, !InitializeThreadLocal(kLocalPageThreadLocalKey)) << "failed to initialize LocalPage ThreadLocalKey";
 }

 void LocalPage::SetForCurrentThread(LocalPage* page) {
   SetCurrentThreadLocal(kLocalPageThreadLocalKey, page);
 }

 LocalPage* LocalPage::GetForCurrentThread() {
   return (LocalPage*) GetCurrentThreadLocal(kLocalPageThreadLocalKey);
 }
}