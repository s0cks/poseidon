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
   friend class SerialScavengerTest;
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
   Semispace fromspace_;
   Semispace tospace_;
   Array<NewPage*> pages_;

   NewZone(const uword start, const word size, const word semi_size):
    Zone(start, size, flags::GetNewPageSize()),
    semisize_(semi_size),
    fromspace_(Space::kFromSpace, GetFromspaceRegion()),
    tospace_(Space::kToSpace, GetTospaceRegion()) {
   }

   inline Semispace& fromspace() {
     return fromspace_;
   }

   inline Semispace& tospace() {
     return tospace_;
   }

   inline Region GetFromspaceRegion() const {
     return Region::Subregion(*this, 0, GetSemispaceSize());
   }

   inline Region GetTospaceRegion() const {
     return Region::Subregion(*this, GetSemispaceSize(), GetSemispaceSize());
   }
  public:
   NewZone() = delete;
   explicit NewZone(const MemoryRegion& region, const RegionSize semi_size = flags::GetNewZoneSemispaceSize()):
    NewZone(region.GetStartingAddress(), region.GetSize(), semi_size) {
   }
   explicit NewZone(const RegionSize size, const RegionSize semi_size = flags::GetNewZoneSemispaceSize()):
    NewZone(MemoryRegion(size), semi_size) {
   }
   NewZone(const NewZone& rhs) = delete;
   ~NewZone() override = default;

   Semispace GetFromspace() const {
     return fromspace_;
   }

   Semispace GetTospace() const {
     return tospace_;
   }

   RegionSize GetSemispaceSize() const {
     return semisize_;
   }

   virtual void SwapSpaces();

   void Clear() override {
     Zone::Clear();
     fromspace_.Clear();
     tospace_.Clear();
   }

   bool IsEmpty() const {
     return fromspace_.IsEmpty();
   }

   virtual Pointer* TryAllocatePointer(word size);
   uword TryAllocateBytes(word size) override;
   uword TryAllocateClassBytes(Class* cls) override;

   template<typename T>
   T* TryAllocate() {
     return (T*) TryAllocateBytes(sizeof(T));
   }

   template<class T>
   T* TryAllocateClass() {
     return (T*) TryAllocateClassBytes(T::GetClass());
   }

   bool VisitPointers(RawObjectVisitor* vis) override {
     return IteratePointers<NewZone, NewZoneIterator>(vis);
   }

   bool VisitMarkedPointers(RawObjectVisitor* vis) override {
     return IterateMarkedPointers<NewZone, NewZoneIterator>(vis);
   }

   bool VisitUnmarkedPointers(RawObjectVisitor* vis) override {
     return IterateUnmarkedPointers<NewZone, NewZoneIterator>(vis);
   }

   bool VisitNewPointers(RawObjectVisitor* vis) override {
     return VisitPointers(vis);
   }

   friend std::ostream& operator<<(std::ostream& stream, const NewZone& val){
     stream << "NewZone(";
     stream << "start=" << val.GetStartingAddressPointer() << ", ";
     stream << "size=" << Bytes(val.GetSize()) << ", ";
     stream << "end=" << val.GetEndingAddressPointer() << ", ";
     stream << "fromspace=" << val.fromspace_ << ", ";
     stream << "tospace=" << val.tospace_  << ", ";
     stream << "semi-size=" << Bytes(val.GetSemispaceSize());
     stream << ")";
     return stream;
   }
  public:
   static NewZone* New(const MemoryRegion& region);

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

   bool PrintSection(NewZone* zone) override {
     LOG_AT_LEVEL(GetSeverity()) << (*zone) << ":";
     return SectionPrinter<NewZone>::PrintSection(zone);
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