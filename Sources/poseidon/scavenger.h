#ifndef POSEIDON_SCAVENGER_H
#define POSEIDON_SCAVENGER_H

#include "poseidon/zone.h"
#include "poseidon/heap.h"
#include "poseidon/stats.h"
#include "poseidon/utils.h"
#include "poseidon/task_pool.h"

namespace poseidon{
 class Heap;
 class LocalPage;
 class Scavenger{
   friend class Allocator;

   template<bool Parallel>
   friend class ScavengerVisitorBase;
   friend class SerialScavengerVisitor;//TODO: cleanup
   friend class ParallelScavengerVisitor;//TODO: cleanup

   friend class ParallelScavengeTask;
  protected:
   NewZone* zone_;
   OldZone* promotion_;
   WorkStealingQueue<uword> work_;
   MinorCollectionStats stats_;

   explicit Scavenger(Heap* heap):
     zone_(heap->new_zone()),
     promotion_(heap->old_zone()),
     work_(),
     stats_(heap->new_zone()->fromspace(), heap->new_zone()->semisize()){
   }

   inline NewZone*
   zone() const{
     return zone_;
   }

   inline void
   SwapSpaces(){
     zone()->SwapSpaces();
   }

   inline WorkStealingQueue<uword>& work(){
     return work_;
   }

   static inline void
   ForwardObject(RawObject* obj, uword forwarding_address){
     GCLOG(1) << "forwarding " << obj->ToString() << " to " << ((RawObject*) forwarding_address)->ToString();
     obj->SetForwardingAddress(forwarding_address);
     PSDN_ASSERT(obj->GetForwardingAddress() == forwarding_address);
   }

   static inline void
   CopyObject(RawObject* src, RawObject* dst){//TODO: create a better copy
  //   PSDN_ASSERT(src->GetTotalSize() == dst->GetTotalSize());
     memcpy(dst->GetPointer(), src->GetPointer(), src->GetPointerSize());
   }

   uword PromoteObject(RawObject* obj);
   uword ScavengeObject(RawObject* obj);
   uword ProcessObject(RawObject* raw);
   void FinalizeObject(RawObject* obj);

   template<class V>
   void ScavengeMemory(){
     SetScavenging();

     V visitor(this);
     visitor.ScavengeMemory();

     ClearScavenging();
   }

   void SerialScavenge();
   void ParallelScavenge();
  public:
   virtual ~Scavenger() = default;

   void Scavenge();
  private:
   static RelaxedAtomic<bool> scavenging_;

   static inline void
   SetScavenging(){
     scavenging_ = true;
   }

   static inline void
   ClearScavenging(){
     scavenging_ = false;
   }

   static inline int32_t
   GetNumberOfWorkers(){
     return FLAGS_num_workers;
   }

   static inline bool
   ShouldUseParallelScavenger(){
     return GetNumberOfWorkers() > 1;
   }
  public:
   static inline bool
   IsScavenging(){
     return (bool)scavenging_;
   }
 };
}

#endif //POSEIDON_SCAVENGER_H
