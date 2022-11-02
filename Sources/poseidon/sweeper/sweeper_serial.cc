#include "poseidon/sweeper/sweeper.h"
#include "poseidon/sweeper/sweeper_serial.h"

namespace poseidon {
 bool SerialSweeper::Visit(RawObject* raw){
   if(raw->IsMarked() || !raw->IsOld() || raw->IsFree())
     return true; // dont sweep marked objects
   return sweeper()->Sweep(raw);
 }

 bool SerialSweeper::SweepPage(Page* page){
   TIMED_SECTION("SweepPage", {
     if(!page->VisitPointers(this))
       return false;
     zone()->Unmark(page->GetIndex());
   });
   return true;
 }
}