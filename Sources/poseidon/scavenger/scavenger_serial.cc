#include "poseidon/local/local_page.h"
#include "poseidon/scavenger/scavenger_serial.h"

namespace poseidon {
 void SerialScavenger::SwapSpaces() {
   DLOG(INFO) << "swapping spaces....";
#ifdef PSDN_DEBUG
   DLOG(INFO) << "semispaces (before):";
   SemispacePrinter::Print(GetFromspace());
   SemispacePrinter::Print(GetTospace());
#endif //PSDN_DEBUG

   new_zone()->SwapSpaces();

#ifdef PSDN_DEBUG
   DLOG(INFO) << "semispaces (after):";
   SemispacePrinter::Print(GetFromspace());
   SemispacePrinter::Print(GetTospace());
#endif //PSDN_DEBUG
 }

 void SerialScavenger::ProcessRoots() {
   SetState(State::kProcessingRoots);
   TIMED_SECTION("ProcessRoots", {
     auto page = GetLocalPageForCurrentThread();
     if(!page->VisitNewPointers(this))
       LOG(FATAL) << "failed to visit new pointers in " << (*page);
   });
 }

 void SerialScavenger::ProcessToSpace() {
   DLOG(INFO) << "processing " << GetTospace() << "....";
   DTIMED_SECTION("ProcessToSpace", {
     if(!GetTospace().VisitPointers(this)) {
       LOG(FATAL) << "failed to process " << GetTospace();
     }
   });
 }

 bool SerialScavenger::Visit(Pointer* ptr) {
   DLOG(INFO) << "visiting " << (*ptr) << "....";
   if(ptr->IsNew() && ptr->IsMarked() && !ptr->IsForwarding()) {
     DLOG(INFO) << "processing " << (*ptr);
     if(ptr->IsRemembered()) {
       auto new_address = Promote(ptr);
     } else{
       auto new_address = Scavenge(ptr);
     }
   }

   if(ptr->VisitPointers(this) != ptr->GetPointerSize()) {
     LOG(ERROR) << "failed to visit pointers in " << (*ptr);
     return false;
   }
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

     DLOG(INFO) << "visiting " << (*ptr);
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

   do {
     ProcessRoots();
   } while(IsProcessingRoots() && HasWork());

   ProcessToSpace();
   LOG_IF(FATAL, !UpdateRoots()) << "failed to update roots.";
 }
}
