#include "poseidon/object.h"
#include "new_zone.h"
#include "poseidon/page/new_page.h"
#include "poseidon/collector/collector.h"

namespace poseidon{
 void NewZone::SwapSpaces() {
   DLOG(INFO) << "swapping semi-spaces....";
#ifdef PSDN_DEBUG
   DLOG(INFO) << "semi-spaces (before):";
   SemispacePrinter::Print(&fromspace());
   SemispacePrinter::Print(&tospace());
#endif //PSDN_DEBUG

   Semispace::Swap(fromspace(), tospace());

#ifdef PSDN_DEBUG
   DLOG(INFO) << "semi-spaces (after):";
   SemispacePrinter::Print(&fromspace());
   SemispacePrinter::Print(&tospace());
#endif //PSDN_DEBUG
 }

 Pointer* NewZone::TryAllocatePointer(const ObjectSize size) {
   if(size < NewZone::GetMinimumObjectSize() || size > NewZone::GetMaximumObjectSize()) {
     PSDN_CANT_ALLOCATE(ERROR, size, (*this));
     return UNALLOCATED;
   }

   Pointer* new_ptr = UNALLOCATED;
   if((new_ptr = fromspace_.TryAllocatePointer(size)) == UNALLOCATED) {
     PSDN_CANT_ALLOCATE(ERROR, size, (*this));
     return UNALLOCATED;
   }
   return new_ptr;
 }

 uword NewZone::TryAllocateBytes(const word size) {
   auto new_ptr = TryAllocatePointer(size);
   if(new_ptr == UNALLOCATED)
     return UNALLOCATED;
   return new_ptr->GetObjectPointerAddress();
 }

 uword NewZone::TryAllocateClassBytes(Class* cls) {
   return TryAllocateBytes(cls->GetAllocationSize());
 }
}