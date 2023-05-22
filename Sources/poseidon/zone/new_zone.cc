#include "poseidon/object.h"
#include "new_zone.h"
#include "poseidon/page/new_page.h"
#include "poseidon/collector/collector.h"

namespace poseidon{
 void NewZone::SwapSpaces() {
   DLOG(INFO) << "swapping semi-spaces....";
#ifdef PSDN_DEBUG
   DLOG(INFO) << "semi-spaces (before):";
   SemispacePrinter::Print(GetFromspace());
   SemispacePrinter::Print(GetTospace());
#endif //PSDN_DEBUG

   auto tmp = from_;
   from_ = to_;
   to_ = tmp;

#ifdef PSDN_DEBUG
   DLOG(INFO) << "semi-spaces (after):";
   SemispacePrinter::Print(GetFromspace());
   SemispacePrinter::Print(GetTospace());
#endif //PSDN_DEBUG
 }

 Pointer* NewZone::TryAllocatePointer(const ObjectSize size) {
   if(size < NewZone::GetMinimumObjectSize() || size > NewZone::GetMaximumObjectSize()) {
     PSDN_CANT_ALLOCATE(ERROR, size, (*this));
     return UNALLOCATED;
   }

   Semispace fromspace = GetFromspace();
   Pointer* new_ptr = UNALLOCATED;
   if((new_ptr = fromspace.TryAllocatePointer(size)) == UNALLOCATED) {
     PSDN_CANT_ALLOCATE(ERROR, size, (*this));
     return UNALLOCATED;
   }
   return new_ptr;
 }

 uword NewZone::TryAllocateBytes(const ObjectSize size) {
   auto new_ptr = TryAllocatePointer(size);
   if(new_ptr == UNALLOCATED)
     return UNALLOCATED;
   return new_ptr->GetObjectPointerAddress();
 }

 uword NewZone::TryAllocateClassBytes(Class* cls) {
   return TryAllocateBytes(cls->GetAllocationSize());
 }
}