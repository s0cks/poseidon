#ifndef POSEIDON_SCAVENGER_H
#define POSEIDON_SCAVENGER_H

#include "poseidon/zone.h"

namespace poseidon{
 class Scavenger : public RawObjectPointerVisitor, public RawObjectPointerPointerVisitor{
  private:
   Zone zone_;
   Semispace from_space_;
   Semispace to_space_;

   /**
    * Copy's an object to the to-space
    *
    * Cheney's Algorithm:
    *
    * copy(o) =
    *   If o has no forwarding address
    *       o' = allocPtr
    *       allocPtr = allocPtr + size(o)
    *       copy the contents of o to o'
    *       forwarding-address(o) = o'
    *   EndIf
    *   return forwarding-address(o)
    *
    * @param raw
    * @return
    */
   uword CopyObject(RawObjectPtr raw);//TODO: create a better copy

   inline void
   MarkObject(RawObject* obj){
     DLOG(INFO) << "marking " << obj->ToString();
     obj->SetMarkedBit();
   }

   inline void
   ForwardObject(RawObject* obj, uword forwarding_address){
     DLOG(INFO) << "forwarding " << obj->ToString() << " to " << ((void*)forwarding_address);
     obj->SetForwardingAddress(forwarding_address);
   }

   uword PromoteObject(RawObject* obj);
   uword ScavengeObject(RawObject* obj);

   void SwapSpaces();
   void ProcessRoots();
   void ProcessToSpace();
   void ProcessCopiedObjects();
  public:
   explicit Scavenger(const Zone& zone):
    zone_(zone),
    from_space_(zone.from()),
    to_space_(zone.to()){
   }
   ~Scavenger() override = default;

   Zone zone() const{
     return zone_;
   }

   Semispace GetToSpace() const{
     return to_space_;
   }

   Semispace GetFromSpace() const{
     return from_space_;
   }

   bool Visit(RawObjectPtr ptr) override;
   bool Visit(RawObjectPtr* ptr) override;

   static void MajorCollection();

   /**
    * Cheney's Algorithm:
    *
    * collect() =
    *   swap(fromspace, tospace)
    *   allocPtr = tospace
    *   scanPtr  = tospace
    *
    *   -- scan every root you've got
    *   ForEach root in the stack -- or elsewhere
    *       root = copy(root)
    *   EndForEach
    *
    *   -- scan objects in the to-space (including objects added by this loop)
    *   While scanPtr < allocPtr
    *       ForEach reference r from o (pointed to by scanPtr)
    *           r = copy(r)
    *       EndForEach
    *       scanPtr = scanPtr  + o.size() -- points to the next object in the to-space, if any
    *   EndWhile
    */
   static void MinorCollection();
 };
}

#endif //POSEIDON_SCAVENGER_H
