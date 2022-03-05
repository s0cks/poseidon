#ifndef POSEIDON_SCAVENGER_H
#define POSEIDON_SCAVENGER_H

#include "poseidon/zone.h"

namespace poseidon{
 class Scavenger : public RawObjectVisitor{
  public:
   struct ScavengerStats{
     RelaxedAtomic<uint64_t> num_scavenged;
     RelaxedAtomic<uint64_t> num_promoted;
     RelaxedAtomic<uint64_t> bytes_scavenged;
     RelaxedAtomic<uint64_t> bytes_promoted;

     ScavengerStats():
       num_scavenged(0),
       num_promoted(0),
       bytes_scavenged(0),
       bytes_promoted(0){
     }
     ~ScavengerStats() = default;
   };

   friend std::ostream& operator<<(std::ostream& stream, const ScavengerStats& stats){
     stream << "ScavengerStats(";
     stream << "scavenged=" << stats.num_scavenged << " (" << HumanReadableSize(stats.bytes_scavenged) << "), ";
     stream << "promoted=" << stats.num_promoted << " (" << HumanReadableSize(stats.bytes_promoted) << ")";
     stream << ")";
     return stream;
   }
  private:
   Zone* zone_;
   Semispace from_space_;
   Semispace to_space_;
   ScavengerStats stats_;


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

     memcpy((void*)dst->GetObjectPointer(), (void*)src->GetObjectPointer(), src->GetPointerSize());
   }

   uword PromoteObject(RawObject* obj);
   uword ScavengeObject(RawObject* obj);
   uword ProcessObject(RawObject* raw);

   void SwapSpaces() const;
   void ProcessRoots();
   void ProcessToSpace() const;
  public:
   explicit Scavenger(Zone* zone):
    zone_(zone),
    from_space_(zone->from()),
    to_space_(zone->to()),
    stats_(){
   }
   ~Scavenger() override = default;

   Zone* zone() const{
     return zone_;
   }

   Semispace GetToSpace() const{
     return to_space_;
   }

   Semispace GetFromSpace() const{
     return from_space_;
   }

   void Scavenge();
   bool Visit(RawObject* val) override;
 };
}

#endif //POSEIDON_SCAVENGER_H
