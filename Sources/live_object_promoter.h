#ifndef POSEIDON_LIVE_OBJECT_PROMOTER_H
#define POSEIDON_LIVE_OBJECT_PROMOTER_H

#include <glog/logging.h>
#include "raw_object.h"

namespace poseidon{
 class LiveObjectPromoter : public RawObjectPointerVisitor{
  private:
   uint64_t promoted_;
   uint64_t scavenged_;

   static inline void
   ForwardObject(RawObject* from, RawObject* to){
     from->SetForwardingAddress((uword)to);
   }

   static inline void
   CopyObject(RawObject* from, RawObject* to){
     //TODO: size checks and faster/better copy?
     DLOG(INFO) << "copying data from " << from->GetPointer() << " to " << to->GetPointer() << "(" << HumanReadableSize(from->GetPointerSize()) << ")";
     memcpy(to->GetPointer(), from->GetPointer(), from->GetPointerSize());
   }

   static inline void
   ForwardAndCopyObject(RawObject* from, RawObject* to){
     ForwardObject(from, to);
     CopyObject(from, to);
   }

   static inline void
   PromoteAndCopyObject(RawObject* obj){
     DLOG(INFO) << "promoting " << obj->ToString() << ".....";
     auto new_ptr = Allocator::GetTenuredHeap()->AllocateRawObject(obj->GetPointerSize());//TODO: use a better allocation function
     new_ptr->SetTenuredBit();
     ForwardAndCopyObject(obj, new_ptr);
   }

   static inline void
   ScavengeObject(RawObject* obj){
     DLOG(INFO) << "scavenging @" << obj << " " << obj->ToString() << ".....";
     auto new_ptr = Allocator::GetEdenHeap()->GetToSpace().AllocateRawObject(obj->GetPointerSize());//TODO: use better allocation function
     new_ptr->SetPointerSize(obj->GetPointerSize());
     new_ptr->SetEdenBit();
     ForwardAndCopyObject(obj, new_ptr);
   }
  public:
   LiveObjectPromoter():
    RawObjectPointerVisitor(),
    promoted_(),
    scavenged_(){
   }
   ~LiveObjectPromoter() override = default;

   uint64_t GetNumberOfPromotedObjects() const{
     return promoted_;
   }

   uint64_t GetNumberOfScavengedObjects() const{
     return scavenged_;
   }

   bool Visit(RawObject* obj) override{
     if(obj->IsReadyForPromotion() && !obj->IsTenured()){
       PromoteAndCopyObject(obj);
       promoted_++;
     } else{
       ScavengeObject(obj);
       scavenged_++;
     }
     return true;
   }
 };
}

#endif//POSEIDON_LIVE_OBJECT_PROMOTER_H