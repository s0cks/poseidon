#include "poseidon/pointer.h"
#include "poseidon/local/local_page.h"
#include "poseidon/platform/os_thread.h"

namespace poseidon {
 bool LocalPage::VisitPointers(RawObjectVisitor* vis) {
   return IteratePointers<LocalPage, LocalPageIterator>(vis);
 }

 bool LocalPage::VisitMarkedPointers(RawObjectVisitor* vis) {
   return IterateMarkedPointers<LocalPage, LocalPageIterator>(vis);
 }

 bool LocalPage::VisitNewPointers(RawObjectVisitor* vis) {
   return IterateNewPointers<LocalPage, LocalPageIterator>(vis);
 }

 bool LocalPage::VisitOldPointers(RawObjectVisitor* vis) {
   return IterateOldPointers<LocalPage, LocalPageIterator>(vis);
 }

 bool LocalPage::Visit(RawObjectPointerVisitor* vis) {
   LocalPagePointerIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(!vis->Visit(next))
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

 static ThreadLocal<LocalPage> kLocalPageThreadLocal("LocalPage");

 void SetLocalPageForCurrentThread(LocalPage* page) {
   kLocalPageThreadLocal.Set(page);
 }

 LocalPage* GetLocalPageForCurrentThread() {
   return kLocalPageThreadLocal.Get();
 }
}