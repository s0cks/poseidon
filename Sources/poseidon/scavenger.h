#ifndef POSEIDON_SCAVENGER_H
#define POSEIDON_SCAVENGER_H

#include "poseidon/zone.h"

namespace poseidon{
 class Heap;
 class LocalGroup;
 class Scavenger{
  protected:
   NewZone* zone_;
   Semispace from_;

   explicit Scavenger(NewZone* zone)://TODO: remove dependency from {@link NewZone}
     zone_(zone),
     from_(zone->from()){
   }

   inline NewZone* zone() const{
     return zone_;
   }

   inline Semispace& to() const{
     return zone()->to();
   }

   inline void SwapSpaces(){
     zone()->SwapSpaces();
   }

   inline void
   ForwardObject(RawObject* obj, uword forwarding_address){
     DLOG(INFO) << "forwarding " << obj->ToString() << " to " << ((RawObject*)forwarding_address)->ToString();
     obj->SetForwardingAddress(forwarding_address);
#ifdef PSDN_DEBUG
     assert(obj->GetForwardingAddress() == forwarding_address);
#endif//PSDN_DEBUG
   }

   static LocalGroup* locals();

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

   virtual void ProcessRoots() = 0;
   virtual void ProcessToSpace() = 0;

   static void SerialScavenge(Heap* heap);
   static void ParallelScavenge(Heap* heap);
  public:
   virtual ~Scavenger() = default;
   virtual void ScavengeMemory() = 0;

   static void Scavenge(Heap* heap);
 };
}

#endif //POSEIDON_SCAVENGER_H
