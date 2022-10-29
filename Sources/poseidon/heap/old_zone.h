#ifndef POSEIDON_HEAP_OLD_ZONE_H
#define POSEIDON_HEAP_OLD_ZONE_H

#include "poseidon/flags.h"
#include "poseidon/bitset.h"
#include "poseidon/heap/zone.h"
#include "poseidon/heap/old_page.h"
#include "poseidon/heap/freelist.h"

namespace poseidon{
 class OldZone : public Zone {
   friend class OldZoneTest;
   friend class SweeperTest;
   friend class Heap;
   friend class Scavenger;

   friend class SerialSweeper;
   friend class ParallelSweeper;
  public:
   class OldZoneIterator : public ZoneIterator<OldZone> {
    public:
     explicit OldZoneIterator(OldZone* zone):
       ZoneIterator<OldZone>(zone) {
     }
     ~OldZoneIterator() override = default;

     bool HasNext() const override {
       return current_address() > 0 &&
              current_address() < zone()->GetEndingAddress() &&
              current_ptr()->IsOld() &&
              current_ptr()->GetSize() > 0;
     }
   };

   static inline ObjectSize
   GetHeaderSize() {
     return sizeof(OldZone);
   }
  protected:
   //TODO: add freelist
   BitSet table_;

   OldZone(const uword start_address, const int64_t& size, const int64_t& num_pages):
    Zone(),
    table_(num_pages) {
   }

   uword TryAllocate(const ObjectSize& size);
   bool InitializePages(const MemoryRegion& region);

   inline bool MarkAllIntersectedBy(const Region& region) {
     NOT_IMPLEMENTED(FATAL); //TODO: implement
     return false;
   }
  public:
   OldZone() = delete;
   ~OldZone() = default; //TODO: change to delete

   uword GetStartingAddress() const override {
     return GetZoneStartingAddress() + GetHeaderSize();
   }

   uword GetZoneStartingAddress() const {
     return (uword)this;
   }

   int64_t GetSize() const override {
     return GetOldZoneSize();
   }

   int64_t GetTotalSize() const {
     return GetHeaderSize() + GetSize();
   }

   FreeList* free_list(){//TODO: visible for testing
     NOT_IMPLEMENTED(FATAL); //TODO: implement
     return nullptr;
   }

   bool VisitPages(OldPageVisitor* vis);
   bool VisitMarkedPages(OldPageVisitor* vis);
   bool VisitPointers(RawObjectVisitor* vis) override;
   bool VisitMarkedPointers(RawObjectVisitor* vis) override;

   friend std::ostream& operator<<(std::ostream& stream, const OldZone& val){
     NOT_IMPLEMENTED(ERROR); //TODO: implement
     return stream;
   }

   static OldZone* From(const MemoryRegion& region);
 };
}

#endif//POSEIDON_HEAP_OLD_ZONE_H