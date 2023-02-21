#include "poseidon/local/local_page.h"
#include "poseidon/scavenger/scavenger_serial.h"

namespace poseidon {
 void SerialScavenger::SwapSpaces() {
   return scavenger()->new_zone()->SwapSpaces();
 }

 bool SerialScavenger::ProcessAll() {
   return ProcessRoots() && ProcessToSpace();
 }

 bool SerialScavenger::ProcessRoots() {
   TIMED_SECTION("ProcessRoots", {
     auto page = GetLocalPageForCurrentThread();
     if(!page->VisitNewPointers(this)){
       LOG(FATAL) << "failed to visit new pointers in " << (*page);
       return false;
     }
   });
   return true;
 }

 bool SerialScavenger::Visit(Pointer* ptr) {
   if(ptr->IsNew() && ptr->IsMarked()) {
     DLOG(INFO) << "visiting " << (*ptr);
     if(ptr->IsForwarding())
       return true;//skip

     DLOG(INFO) << "processing " << (*ptr);
     if(ptr->IsRemembered()) {
       auto new_address = Promote(ptr);
     } else{
       auto new_address = Scavenge(ptr);
     }

     if((ptr->VisitPointers(this) != ptr->GetPointerSize())) {
       LOG(ERROR) << "failed to visit pointers in " << (*ptr);
       return false;
     }

     ptr->SetRemembered(true);
   }
   return true;
 }

 bool SerialScavenger::ProcessToSpace() {
   DLOG(INFO) << "tospace:";
   SemispacePrinter::Print(&tospace());
   return true;
 }

 class SerialRootNotifier : public RawObjectPointerVisitor {
  protected:
   static inline bool Update(Pointer** ptr, uword new_address) {
     DLOG(INFO) << "updating " << (**ptr) << " to " << *((Pointer*)new_address);
     (*ptr) = (Pointer*)new_address;
     return true;
   }
  public:
   SerialRootNotifier() = default;
   ~SerialRootNotifier() override = default;

   bool Visit(Pointer** raw_ptr) override {
     auto ptr = (*raw_ptr);
     if(ptr == UNALLOCATED)
       return false;

     if(!ptr->IsNew() || !ptr->IsForwarding()) {
       return false;
     }
     return Update(raw_ptr, ptr->GetForwardingAddress());
   }
 };

 bool SerialScavenger::UpdateRoots() {
   SerialRootNotifier notifier;
   TIMED_SECTION("SerialScavenger::UpdateRoots", {
     auto page = GetLocalPageForCurrentThread();
     LOG_IF(FATAL, !page->Visit(&notifier)) << "failed to notify roots";
   });
   return true;
 }

 void SerialScavenger::ScavengeMemory() {
   LOG_IF(FATAL, !LocalPageExistsForCurrentThread()) << "no local page exists for current thread.";
   SwapSpaces();
   LOG_IF(FATAL, !ProcessRoots()) << "failed to process roots.";
   LOG_IF(FATAL, !ProcessToSpace()) << "failed to process to-space.";
   LOG_IF(FATAL, !UpdateRoots()) << "failed to update roots.";
 }
}
