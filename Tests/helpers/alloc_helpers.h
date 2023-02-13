#ifndef POSEIDON_ALLOC_HELPERS_H
#define POSEIDON_ALLOC_HELPERS_H

#include "poseidon/pointer.h"
#include "poseidon/marker/marker.h"

namespace poseidon {
#ifndef UNALLOCATED
#define UNALLOCATED 0 //TODO: cleanup
#endif // UNALLOCATED

 template<class Z>
 static inline uword
 TryAllocateBytes(Z* zone, const ObjectSize size) {
   return zone->TryAllocateBytes(size);
 }

 template<class Z>
 static inline Pointer*
 TryAllocateWord(Z* zone, const word value) {
   auto address = TryAllocateBytes<Z>(zone, kWordSize);
   if (address == UNALLOCATED)
     return nullptr;
   auto ptr = (Pointer*)address;
   (*((word*)ptr->GetObjectPointerAddress())) = value;
   return ptr;
 }

 template<class Z>
 static inline Pointer*
 TryAllocateMarkedWord(Z* zone, const word value) {
   auto address = TryAllocateBytes<Z>(zone, kWordSize);
   if (address == UNALLOCATED)
     return nullptr;
   Marker marker;
   auto ptr = (Pointer*)address;
   marker.Visit(ptr);
   (*((word*)ptr->GetObjectPointerAddress())) = value;
   return ptr;
 }

 template<class Z>
 static inline Pointer*
 TryAllocateRememberedWord(Z* zone, const word value) {
   auto address = TryAllocateBytes<Z>(zone, kWordSize);
   if (address == UNALLOCATED)
     return nullptr;
   auto ptr = (Pointer*)address;
   //TODO: ptr->SetRememberedBit();
   (*((word*)ptr->GetObjectPointerAddress())) = value;
   return ptr;
 }
}

#endif // POSEIDON_ALLOC_HELPERS_H