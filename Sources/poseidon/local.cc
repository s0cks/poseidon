#include "poseidon/local.h"

namespace poseidon{//TODO: clean-up this code
 ThreadLocalKey LocalPage::kThreadKey = PTHREAD_KEYS_MAX;

 LocalBase::LocalBase():
  previous_(nullptr),
  next_(nullptr),
  slot_(nullptr){
   auto page = LocalPage::GetLocalPageForCurrentThread();
   if(page != nullptr)
     slot_ = page->GetFirstLocalSlotAvailable();
 }

 void LocalPage::VisitObjects(RawObjectVisitor* vis) const{
   LocalPageIterator iter(this);
   while(iter.HasNext()){
     auto next = iter.Next();
     if(!vis->Visit(next))
       return;
   }
 }

 void LocalPage::VisitObjects(const std::function<bool(RawObject*)>& vis) const{
   LocalPageIterator iter(this);
   while(iter.HasNext()){
     auto next = iter.Next();
     if(!vis(next))
       return;
   }
 }

 void LocalPage::VisitPointers(RawObjectPointerVisitor* vis) const{
   LocalPageIterator iter(this);
   while(iter.HasNext()){
     auto next = iter.NextPointer();
     if(!vis->Visit(next))
       return;
   }
 }

 void LocalPage::VisitPointers(const std::function<bool(RawObject**)>& vis) const{
   LocalPageIterator iter(this);
   while(iter.HasNext()){
     auto next = iter.NextPointer();
     if((*next) && (*next)->GetPointerSize() > 0 && !vis(next))
       return;
   }
 }
}
