#include "poseidon/local/local_page.h"
#include "poseidon/platform/os_thread.h"
#include "poseidon/marker/marker_serial.h"

namespace poseidon {
 void SerialMarker::MarkAllRoots() {
   ClearStats();

   LOG_IF(FATAL, !LocalPageExistsForCurrentThread()) << "no local page exists for current thread.";
   auto page = GetLocalPageForCurrentThread();
   LOG_IF(FATAL, !page->VisitPointers(this)) << "failed to visit pointers in " << (*page);

#ifdef PSDN_DEBUG
   DLOG(INFO) << GetStats();
#endif //PSDN_DEBUG
 }

 bool SerialMarker::Visit(Pointer* ptr) {
   if(ptr->IsMarked())
     return true; // skip

   Mark(ptr);
   return true;
 }
}