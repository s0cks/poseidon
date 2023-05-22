#ifndef POSEIDON_HEAP_OLD_ZONE_H
#define POSEIDON_HEAP_OLD_ZONE_H

#include "poseidon/flags.h"
#include "poseidon/bitset.h"
#include "zone.h"
#include "poseidon/page/old_page.h"
#include "poseidon/freelist/freelist.h"

namespace poseidon{
 class OldZone : public Zone {
   friend class OldZoneTest;
   friend class SweeperTest;
   friend class Heap;
   friend class Scavenger;

   friend class SerialSweeper;
   friend class ParallelSweeper;
   friend class SerialSweeperTest;
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

   explicit OldZone(const MemoryRegion& region):
    Zone(region),
    free_list_(region, false) {
   }

   OldZone():
    OldZone(MemoryRegion(flags::GetOldZoneSize())) {
   }
  public:
   ~OldZone() override = default;
   DEFINE_NON_COPYABLE_TYPE(OldZone);

   inline FreeList& free_list() { //TODO: visible for testing
     return free_list_;
   }

   Pointer* TryAllocatePointer(word size);
   uword TryAllocateBytes(word size) override;
   uword TryAllocateClassBytes(Class* cls) override;

   bool VisitPointers(RawObjectVisitor* vis) const override {
     return IteratePointers<OldZone, OldZoneIterator>(vis);
   }

   bool VisitMarkedPointers(RawObjectVisitor* vis) const override {
     return IterateMarkedPointers<OldZone, OldZoneIterator>(vis);
   }

   bool VisitUnmarkedPointers(RawObjectVisitor* vis) const override {
     return IterateUnmarkedPointers<OldZone, OldZoneIterator>(vis);
   }

   bool VisitNewPointers(RawObjectVisitor* vis) const override {
     return false; // does not compute
   }

   bool VisitOldPointers(RawObjectVisitor* vis) const override {
     NOT_IMPLEMENTED(ERROR); //TODO: implement
     return false;
   }

   void ClearOldZone() {
     free_list().ClearFreeList();
     free_list().Insert((const Region&)*this);
   }

   friend std::ostream& operator<<(std::ostream& stream, const OldZone& val){
     stream << "OldZone(";
     stream << "start=" << val.GetStartingAddressPointer() << ", ";
     stream << "size=" << Bytes(val.GetSize()) << ", ";
     stream << "end=" << val.GetEndingAddressPointer();
     stream << ")";
     return stream;
   }
  public:
   static constexpr ObjectSize
   GetMinimumObjectSize() {
     return kWordSize;
   }

   static ObjectSize
   GetMaximumObjectSize() {
     return flags::GetLargeObjectSize();
   }

   static inline OldZone*
   New() {
     return new OldZone();
   }
 };

 class OldZonePrinter : public SectionPrinter<OldZone> {
  protected:
   explicit OldZonePrinter(const google::LogSeverity severity):
    SectionPrinter<OldZone>(severity) {
   }
  public:
   ~OldZonePrinter() override = default;
  public:
   template<const google::LogSeverity Severity = google::INFO>
   static inline bool
   Print(OldZone* zone) {
     OldZonePrinter printer(Severity);
     if(!printer.Start()) {
       LOG_AT_LEVEL(Severity) << "printer.Start() failed.";
       return false;
     }
   }

   template<const google::LogSeverity Severity = google::INFO>
   static inline bool
   Print(const OldZone& zone) {
     return Print<Severity>(&zone);
   }
 };
}

#endif//POSEIDON_HEAP_OLD_ZONE_H