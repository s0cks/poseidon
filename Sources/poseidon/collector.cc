#include <glog/logging.h>

#include "poseidon/utils.h"
#include "poseidon/marker.h"
#include "poseidon/allocator.h"
#include "poseidon/collector.h"
#include "poseidon/scavenger.h"

namespace poseidon{
/**
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
 void Collector::MinorCollection(){
   DLOG(INFO) << "executing a minor collection....";
   auto heap = Allocator::GetHeap();
   auto zone = heap->new_zone();
   Scavenger scavenger(zone);

#ifdef PSDN_DEBUG
   auto start_ts = Clock::now();
#endif//PSDN_DEBUG

   scavenger.Scavenge();

#ifdef PSDN_DEBUG
   auto duration = Clock::now() - start_ts;
   DLOG(INFO) << "minor collection has finished in " << duration << ".";
#endif//PSDN_DEBUG
 }

 void Collector::MajorCollection(){
   DLOG(INFO) << "executing major collection....";
   auto heap = Allocator::GetHeap();
   auto zone = heap->old_zone();

#ifdef PSDN_DEBUG
   auto start_ts = Clock::now();
#endif//PSDN_DEBUG

   // 1. Clean Old Zone

   // 2. Clean HeapPages

#ifdef PSDN_DEBUG
   auto duration = (Clock::now() - start_ts);
   DLOG(INFO) << "major collection has finished in " << duration << ".";
#endif//PSDN_DEBUG
 }
}