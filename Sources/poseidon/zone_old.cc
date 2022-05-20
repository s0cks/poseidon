#include "poseidon/zone_old.h"

namespace poseidon{
 OldPageTable::OldPageTable(int64_t num_pages):
  pages_(new OldPage[num_pages]),
  num_pages_(num_pages),
  marked_(num_pages){
   PSDN_ASSERT(IsPow2(num_pages));
 }

 OldPageTable::OldPageTable(const OldPageTable& rhs):
  pages_(new OldPage[rhs.size()]),
  num_pages_(rhs.size()),
  marked_(rhs.marked_){
   std::copy(begin(), end(), rhs.begin());
 }

 uword OldZone::TryAllocate(int64_t size){
   auto ptr = (void*)free_list_.TryAllocate(size);
   auto val = new (ptr)RawObject();
   val->SetOldBit();
   val->SetPointerSize(size);//TODO: mark page
   return val->GetAddress();
 }
}