#ifndef POSEIDON_HEAP_OLD_ZONE_H
#define POSEIDON_HEAP_OLD_ZONE_H

#include "poseidon/flags.h"
#include "poseidon/bitset.h"
#include "poseidon/heap/zone/zone.h"
#include "poseidon/heap/page/old_page.h"
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
   class OldZoneIterator : public ZonePointerIterator {
    public:
     explicit OldZoneIterator(const OldZone* zone):
       ZonePointerIterator(zone) {
     }
     ~OldZoneIterator() override = default;

     bool HasNext() const override {
       return current_address() >= zone()->GetStartingAddress() &&
           current_address() < zone()->GetEndingAddress() &&
           current_ptr()->IsOld() &&
           !current_ptr()->IsFree() &&
           current_ptr()->GetSize() > 0;
     }
   };
  protected:
   FreeList free_list_;

   OldZone(const uword start_address, const int64_t size):
    Zone(start_address, size),
    free_list_(start_address, size) {
     SetWritable();
     free_list_.Insert(start_address, size);
   }
  public:
   OldZone() = delete;
   explicit OldZone(const MemoryRegion& region):
    OldZone(region.GetStartingAddress(), region.GetSize()) {
   }
   explicit OldZone(const int64_t size):
    OldZone(MemoryRegion(size)) {
   }
   OldZone(const OldZone& rhs) = delete;
   ~OldZone() override = default;

   inline FreeList& free_list() { //TODO: visible for testing
     return free_list_;
   }

   Pointer* TryAllocatePointer(word size);
   uword TryAllocateBytes(word size) override;
   uword TryAllocateClassBytes(Class* cls) override;

   bool VisitPointers(RawObjectVisitor* vis) override {
     return IteratePointers<OldZone, OldZoneIterator>(vis);
   }

   bool VisitPointers(const std::function<bool(Pointer*)>& vis) override {
     return IteratePointers<OldZone, OldZoneIterator>(vis);
   }

   bool VisitMarkedPointers(RawObjectVisitor* vis) override {
     return IterateMarkedPointers<OldZone, OldZoneIterator>(vis);
   }

   bool VisitMarkedPointers(const std::function<bool(Pointer*)>& vis) override {
     return IterateMarkedPointers<OldZone, OldZoneIterator>(vis);
   }

   friend std::ostream& operator<<(std::ostream& stream, const OldZone& val){
     stream << "OldZone(";
     stream << "start=" << val.GetStartingAddressPointer() << ", ";
     stream << "size=" << Bytes(val.GetSize()) << ", ";
     stream << "end=" << val.GetEndingAddressPointer();
     stream << ")";
     return stream;
   }

   OldZone& operator=(const OldZone& rhs) = delete;
  public:
   static constexpr ObjectSize
   GetMinimumObjectSize() {
     return kWordSize;
   }

   static ObjectSize
   GetMaximumObjectSize() {
     return flags::GetLargeObjectSize();
   }
 };
}

#endif//POSEIDON_HEAP_OLD_ZONE_H