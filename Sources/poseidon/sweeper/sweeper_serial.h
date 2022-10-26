#ifndef POSEIDON_SERIAL_SWEEPER_H
#define POSEIDON_SERIAL_SWEEPER_H

#include "poseidon/heap/freelist.h"
#include "poseidon/collector/sweeper.h"
#include "poseidon/sweeper/sweeper_base.h"

namespace poseidon {
 class SerialSweeper : public SweeperBase<false> {
  protected:
   FreeList* free_list_;

   inline FreeList* free_list() const {
     return free_list_;
   }

   bool Visit(RawObject* ptr) override {
     if(ptr->IsMarked())
       return true; // don't sweep marked objects
     Sweeper::SweepObject(free_list(), ptr);
     return true;
   }

   bool VisitPage(Page* page) override {
     auto current = page->GetStartingAddress();
     while(current < page->GetEndingAddress() && ((RawObject*)current)->GetPointerSize() > 0){
       auto ptr = (RawObject*)current;
       auto size = ptr->GetTotalSize();

       if(!Visit(ptr))
         return false;

       current += size;
     }
     return true;
   }
  public:
   SerialSweeper() = default;
   ~SerialSweeper() override = default;
 };
}

#endif // POSEIDON_SERIAL_SWEEPER_H