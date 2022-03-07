#ifndef POSEIDON_COLLECTOR_H
#define POSEIDON_COLLECTOR_H

#include "poseidon/utils.h"
#include "poseidon/relaxed_atomic.h"

namespace poseidon{
 struct CollectionStats{
   RelaxedAtomic<uint64_t> num_marked;

   CollectionStats():
     num_marked(0){
   }
   CollectionStats(const CollectionStats& rhs) = default;
   virtual ~CollectionStats() = default;

   virtual void reset(){
     num_marked = 0;
   }

   CollectionStats& operator=(const CollectionStats& rhs) = default;

   friend std::ostream& operator<<(std::ostream& stream, const CollectionStats& val){
     stream << "CollectionStats:" << std::endl;
     stream << " * Number of Objects Marked: " << val.num_marked << std::endl;
     return stream;
   }
 };

 struct MinorCollectionStats : public CollectionStats{
   RelaxedAtomic<uint64_t> num_promoted;
   RelaxedAtomic<uint64_t> num_scavenged;
   RelaxedAtomic<uint64_t> bytes_promoted;
   RelaxedAtomic<uint64_t> bytes_scavenged;

   MinorCollectionStats():
     CollectionStats(),
     num_promoted(0),
     num_scavenged(0),
     bytes_promoted(0),
     bytes_scavenged(0){
   }
   ~MinorCollectionStats() override = default;

   void reset() override{
     CollectionStats::reset();
     num_promoted = 0;
     num_scavenged = 0;
     bytes_promoted = 0;
     bytes_scavenged = 0;
   }

   friend std::ostream& operator<<(std::ostream& stream, const MinorCollectionStats& val){
     stream << "MinorCollectionStats:" << std::endl;
     stream << " * Number of Objects Marked: " << val.num_marked << std::endl;
     stream << " * Number of Objects Promoted: " << val.num_promoted << " (" << HumanReadableSize(val.bytes_promoted) << ")" << std::endl;
     stream << " * Number of Objects Scavenged: " << val.num_scavenged << " (" << HumanReadableSize(val.bytes_scavenged) << ")" << std::endl;
     return stream;
   }
 };

 struct MajorCollectionStats : public CollectionStats{
   RelaxedAtomic<uint64_t> num_reclaimed;
   RelaxedAtomic<uint64_t> bytes_reclaimed;

   MajorCollectionStats():
    CollectionStats(),
    num_reclaimed(0),
    bytes_reclaimed(0){
   }
   ~MajorCollectionStats() override = default;

   void reset() override{
     CollectionStats::reset();
     num_reclaimed = 0;
     bytes_reclaimed = 0;
   }

   friend std::ostream& operator<<(std::ostream& stream, const MajorCollectionStats& val){
     stream << "MajorCollectionStats:" << std::endl;
     stream << " * Number of Objects Marked: " << val.num_marked << std::endl;
     stream << " * Number of Objects Reclaimed: " << val.num_reclaimed << " (" << HumanReadableSize(val.bytes_reclaimed) << ") " << std::endl;
     return stream;
   }
 };

 class Collector{
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
  private:
   static void CompactOldZone();
   static void CompactHeapPages();
  public:
   Collector() = delete;
   Collector(const Collector& rhs) = delete;
   ~Collector() = delete;

   /**
    * Cleans the New {@link Zone} of the {@link Heap}.
    */
   static void MinorCollection();

   /**
    * Cleans both the New & Old {@link Zone}s of the {@link Heap}.
    */
   static void MajorCollection();

   Collector& operator=(const Collector& rhs) = delete;
 };
}

#endif//POSEIDON_COLLECTOR_H