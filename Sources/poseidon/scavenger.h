#ifndef POSEIDON_SCAVENGER_H
#define POSEIDON_SCAVENGER_H

#include "poseidon/zone.h"

namespace poseidon{
 class Scavenger{
  private:
   inline void
   ForwardObject(RawObject* obj, uword forwarding_address){
     DLOG(INFO) << "forwarding " << obj->ToString() << " to " << ((RawObject*)forwarding_address)->ToString();
     obj->SetForwardingAddress(forwarding_address);
#ifdef PSDN_DEBUG
     assert(obj->GetForwardingAddress() == forwarding_address);
#endif//PSDN_DEBUG
   }

   inline void
   CopyObject(RawObject* src, RawObject* dst){//TODO: create a better copy
#ifdef PSDN_DEBUG
     assert(src->GetTotalSize() == dst->GetTotalSize());
#endif//PSDN_DEBUG
     memcpy(dst->GetPointer(), src->GetPointer(), src->GetPointerSize());
   }

   uword PromoteObject(RawObject* obj);
   uword ScavengeObject(RawObject* obj);
   uword ProcessObject(RawObject* raw);

   void SwapSpaces() const;
   void ProcessRoots();
   void ProcessToSpace() const;
  public:
   Scavenger() = default;
   ~Scavenger() = default;
   void Scavenge();
 };
}

#endif //POSEIDON_SCAVENGER_H
