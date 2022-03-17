#include "poseidon/utils.h"
#include "poseidon/local.h"
#include "poseidon/allocator.h"
#include "poseidon/os_thread.h"

namespace poseidon{
 static pthread_key_t kHeapThreadLocal;

 Allocator::RootPage* Allocator::roots_ = nullptr;

 void Allocator::Initialize(){
   roots_ = new RootPage();
 }

 void Allocator::InitializeForThread(){//TODO: rename
   int err;
   if((err = pthread_key_create(&kHeapThreadLocal, NULL)) != 0){
     LOG(ERROR) << "failed to create heap thread local: " << strerror(err);
     return;
   }
 }

 void Allocator::VisitRoots(const std::function<bool(uword, uword)>& vis){
   auto page = roots_;
   while(page != nullptr){
     page->VisitPointers(vis);
     page = page->GetNext();
   }
 }
}