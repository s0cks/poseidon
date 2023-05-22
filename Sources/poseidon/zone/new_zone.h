#ifndef POSEIDON_HEAP_NEW_ZONE_H
#define POSEIDON_HEAP_NEW_ZONE_H

#include "poseidon/flags.h"
#include "poseidon/bitset.h"
#include "poseidon/zone/zone.h"
#include "poseidon/page/new_page.h"
#include "poseidon/heap/semispace.h"

namespace poseidon{
 class NewZone : public Zone { //TODO: make resizable
   template<bool Parallel>
   friend class ScavengerVisitorBase;//TODO: remove

   friend class Zone;

   friend class Heap;
   friend class NewZoneTest;
   friend class Scavenger;
   friend class SerialScavenger;
   friend class SerialMarkerTest;
   friend class SerialScavengerTest;
   friend class PageTableTest;
   friend class UInt8Test;
   friend class ParallelMarkerTest;

   friend class BaseNewZoneTest;
  public:
   class NewZoneIterator : public ZonePointerIterator {
    public:
     explicit NewZoneIterator(const NewZone* zone):
       ZonePointerIterator(zone) {
     }
     ~NewZoneIterator() override = default;

     bool HasNext() const override {
       return zone()->Contains(current_address()) &&
              current_ptr()->IsNew() &&
              !current_ptr()->IsFree() &&
              current_ptr()->GetSize() > 0;
     }
   };
  protected:
   word semisize_;
   uword from_;
   uword to_;
   Array<NewPage*> pages_;

   explicit NewZone(const MemoryRegion& region, const RegionSize semi_size = flags::GetNewZoneSemispaceSize()):
    Zone(region),
    semisize_(semi_size),
    from_(region.GetStartingAddress()),
    to_(region.GetStartingAddress() + semi_size),
    pages_() {
   }
   NewZone():
    NewZone(MemoryRegion(flags::GetNewZoneSize())) {
   }

   inline uword fromspace() const {
     return from_;
   }

   inline uword tospace() const {
     return to_;
   }
  public:
   ~NewZone() override = default;
   DEFINE_NON_COPYABLE_TYPE(NewZone);

   Semispace GetFromspace() const {
     return Semispace(Space::kFromSpace, GetStartingAddress(), fromspace(), GetSemispaceSize());
   }

   Semispace GetTospace() const {
     return Semispace(Space::kToSpace, GetStartingAddress() + GetSemispaceSize(), tospace(), GetSemispaceSize());
   }

   RegionSize GetSemispaceSize() const {
     return semisize_;
   }

   virtual void SwapSpaces();

   void Clear() override {
     Zone::Clear();
     from_ = GetStartingAddress();
     to_ = GetStartingAddress() + GetSemispaceSize();
   }

   virtual bool IsEmpty() const {
     return fromspace() == GetStartingAddress();
   }

   virtual Pointer* TryAllocatePointer(word size);
   uword TryAllocateBytes(const ObjectSize size) override;
   uword TryAllocateClassBytes(Class* cls) override;

   template<typename T>
   T* TryAllocate() {
     return (T*) TryAllocateBytes(sizeof(T));
   }

   template<class T>
   T* TryAllocateClass() {
     return (T*) TryAllocateClassBytes(T::GetClass());
   }

   bool VisitPointers(RawObjectVisitor* vis) const override {
     return IteratePointers<NewZone, NewZoneIterator>(vis);
   }

   bool VisitMarkedPointers(RawObjectVisitor* vis) const override {
     return IterateMarkedPointers<NewZone, NewZoneIterator>(vis);
   }

   bool VisitUnmarkedPointers(RawObjectVisitor* vis) const override {
     return IterateUnmarkedPointers<NewZone, NewZoneIterator>(vis);
   }

   bool VisitNewPointers(RawObjectVisitor* vis) const override {
     return VisitPointers(vis);
   }

   friend std::ostream& operator<<(std::ostream& stream, const NewZone& val){
     stream << "NewZone(";
     stream << "start=" << val.GetStartingAddressPointer() << ", ";
     stream << "size=" << Bytes(val.GetSize()) << ", ";
     stream << "end=" << val.GetEndingAddressPointer() << ", ";
     stream << "fromspace=" << val.fromspace() << ", ";
     stream << "tospace=" << val.tospace()  << ", ";
     stream << "semi-size=" << Bytes(val.GetSemispaceSize());
     stream << ")";
     return stream;
   }
  public:
   static NewZone* New(const Region& region);

   static inline RegionSize
   GetNewZoneSize() {
     return flags::GetNewZoneSize();
   }

   static constexpr word
   GetMinimumObjectSize() {
     return kWordSize;
   }

   static constexpr word
   GetMaximumObjectSize() {
     return Semispace::GetMaximumObjectSize();
   }
 };

 class NewZonePrinter : public SectionPrinter<NewZone> {
  protected:
   explicit NewZonePrinter(const google::LogSeverity severity):
     SectionPrinter<NewZone>(severity) {
   }

   bool PrintSection(NewZone* zone) {
     LOG_AT_LEVEL(GetSeverity()) << (*zone) << ":";
     return zone->VisitPointers(this);
   }
  public:
   ~NewZonePrinter() override = default;
  public:
   template<const google::LogSeverity Severity = google::INFO>
   static inline bool
   Print(NewZone* zone) {
     NewZonePrinter printer(Severity);
     return printer.PrintSection(zone);
   }
 };
}

#endif//POSEIDON_HEAP_NEW_ZONE_H