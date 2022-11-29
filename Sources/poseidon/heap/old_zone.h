#ifndef POSEIDON_HEAP_OLD_ZONE_H
#define POSEIDON_HEAP_OLD_ZONE_H

#include "poseidon/flags.h"
#include "poseidon/bitset.h"
#include "poseidon/heap/zone.h"
#include "poseidon/heap/old_page.h"
#include "poseidon/heap/freelist.h"

namespace poseidon{
 class OldZone : public Zone<OldPage> {
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
       return current_address() >= zone()->GetStartingAddress() &&
              current_address() < zone()->GetEndingAddress() &&
              current_ptr()->IsOld() &&
              current_ptr()->GetSize() > 0 &&
              !current_ptr()->IsFree();
     }
   };

   class OldZonePageIterator : public ZonePageIterator {
    public:
     explicit OldZonePageIterator(OldZone* zone):
       ZonePageIterator(zone) {
     }
     ~OldZonePageIterator() override = default;
   };
  protected:
   FreeList free_list_;

   OldZone(const uword start_address, const int64_t size):
    Zone<OldPage>(start_address, size, GetOldPageSize()),
    free_list_(start_address, size) {
     SetWritable();
     free_list_.Insert(start_address, size);
   }

   virtual bool MarkAllIntersectedBy(const Region& region);
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

   uword TryAllocateBytes(ObjectSize size);

   inline uword TryAllocate(const ObjectSize size) {
     return TryAllocateBytes(size); //TODO: remove
   }

   virtual bool VisitPages(OldPageVisitor* vis) {
     OldZonePageIterator iter(this);
     while(iter.HasNext()) {
       auto next = iter.Next();
       if(!vis->Visit(next))
         return false;
     }
     return true;
   }

   virtual bool VisitMarkedPages(OldPageVisitor* vis) {
     OldZonePageIterator iter(this);
     while(iter.HasNext()) {
       auto next = iter.Next();
       if(IsMarked(next) && !vis->Visit(next))
           return false;
     }
     return true;
   }

   bool VisitPointers(RawObjectVisitor* vis) override;
   bool VisitPointers(std::function<bool(Pointer*)>& function);
   bool VisitMarkedPointers(RawObjectVisitor* vis) override;
   bool VisitMarkedPointers(std::function<bool(Pointer*)>& function);

   friend std::ostream& operator<<(std::ostream& stream, const OldZone& val){
     stream << "OldZone(";
     stream << "start=" << val.GetStartingAddressPointer() << ", ";
     stream << "size=" << Bytes(val.GetSize()) << ", ";
     stream << "end=" << val.GetEndingAddressPointer();
     stream << ")";
     return stream;
   }

   static OldZone* From(const MemoryRegion& region);

   static constexpr ObjectSize
   GetMinimumObjectSize() {
     return kWordSize;
   }

   static ObjectSize
   GetMaximumObjectSize() {
     return GetLargeObjectSize();
   }
 };
}

#endif//POSEIDON_HEAP_OLD_ZONE_H