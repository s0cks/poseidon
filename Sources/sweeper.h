#ifndef POSEIDON_SWEEPER_H
#define POSEIDON_SWEEPER_H

#include <glog/logging.h>
#include "heap.h"

namespace poseidon{
 class Sweeper{
  protected:
   Sweeper() = default;
  public:
   virtual ~Sweeper() = default;
   virtual void Sweep(Heap* heap) = 0;
 };

 class GenerationalSweeper : public Sweeper{
  public:
   GenerationalSweeper() = default;
   ~GenerationalSweeper() override = default;

   void Sweep(Heap* heap) override{
     DLOG(INFO) << "sweeping the " << heap->GetSpace() << " heap....";
     heap->GetFromSpace().Clear();
     heap->SwapSpaces();
   }

   static inline void
   SweepHeap(Heap* heap){
     GenerationalSweeper sweeper;
     return sweeper.Sweep(heap);
   }
 };

 class MarkCopySweeper : public Sweeper{
  public:
   MarkCopySweeper() = default;
   ~MarkCopySweeper() override = default;

   void Sweep(Heap* heap) override{

   }
 };
}

#endif//POSEIDON_SWEEPER_H
