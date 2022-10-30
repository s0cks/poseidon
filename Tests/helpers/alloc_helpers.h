#ifndef POSEIDON_ALLOC_HELPERS_H
#define POSEIDON_ALLOC_HELPERS_H

#include "poseidon/raw_object.h"

namespace poseidon {
#ifndef UNALLOCATED
#define UNALLOCATED 0 //TODO: cleanup
#endif // UNALLOCATED

 template<class Z>
 static inline uword
 TryAllocateBytes(Z* zone, const ObjectSize size) {
   return zone->TryAllocate(size);
 }

 template<class Z>
 static inline RawObject*
 TryAllocateWord(Z* zone, const word value) {
   auto address = TryAllocateBytes<Z>(zone, kWordSize);
   if (address == UNALLOCATED)
     return nullptr;
   auto ptr = (RawObject*)address;
   (*((word*)ptr->GetObjectPointerAddress())) = value;
   return ptr;
 }

 template<class Z>
 static inline RawObject*
 TryAllocateMarkedWord(Z* zone, const word value) {
   auto address = TryAllocateBytes<Z>(zone, kWordSize);
   if (address == UNALLOCATED)
     return nullptr;
   auto ptr = (RawObject*)address;
   ptr->SetMarkedBit();
   (*((word*)ptr->GetObjectPointerAddress())) = value;
   return ptr;
 }
}

#endif // POSEIDON_ALLOC_HELPERS_H