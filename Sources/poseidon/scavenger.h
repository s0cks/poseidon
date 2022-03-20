#ifndef POSEIDON_SCAVENGER_H
#define POSEIDON_SCAVENGER_H

#include "poseidon/zone.h"
#include "poseidon/heap.h"

namespace poseidon{
 class Heap;
 class LocalGroup;
 class Scavenger{
   template<bool Parallel>
   friend class ScavengerVisitorBase;

   friend class ParallelScavengeTask;
  protected:
   NewZone* zone_;
   Semispace from_;

   RelaxedAtomic<int64_t> num_promoted_;
   RelaxedAtomic<int64_t> bytes_promoted_;
   RelaxedAtomic<int64_t> num_scavenged_;
   RelaxedAtomic<int64_t> bytes_scavenged_;

   explicit Scavenger(NewZone* zone)://TODO: remove dependency from {@link NewZone}
     zone_(zone),
     from_(zone->from()),
     num_promoted_(0),
     bytes_promoted_(0),
     num_scavenged_(0),
     bytes_scavenged_(0){
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
     DVLOG(1) << "forwarding " << obj->ToString() << " to " << ((RawObject*)forwarding_address)->ToString();
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
  public:
   virtual ~Scavenger() = default;

   void Scavenge();

   static inline void
   ScavengeFromCurrentThreadHeap(){
     Scavenger scavenger(Heap::GetCurrentThreadHeap()->new_zone());
     scavenger.Scavenge();
   }
 };
}

#endif //POSEIDON_SCAVENGER_H
