#ifndef POSEIDON_SCAVENGER_H
#define POSEIDON_SCAVENGER_H

#include "raw_object.h"

namespace poseidon{
 class Heap;
 class Scavenger : public RawObjectPointerVisitor, public RawObjectPointerPointerVisitor{
  private:
   Heap* heap_;
   Semispace& to_space_;

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

   void ProcessRoots();
   void ProcessToSpace();
   void ProcessCopiedObjects();
  public:
   explicit Scavenger(Heap* heap):
    heap_(heap),
    to_space_(heap->GetToSpace()){
   }
   ~Scavenger() override = default;

   Heap* GetHeap() const{
     return heap_;
   }

   Semispace& GetToSpace() const{
     return to_space_;
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
