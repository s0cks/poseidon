#ifndef POSEIDON_HEAP_NEW_ZONE_H
#define POSEIDON_HEAP_NEW_ZONE_H

#include "poseidon/flags.h"
#include "poseidon/bitset.h"
#include "zone.h"
#include "poseidon/heap/page/new_page.h"
#include "poseidon/heap/semispace.h"

namespace poseidon{
 class NewZone : public Zone { //TODO: make resizable
   template<bool Parallel>
   friend class ScavengerVisitorBase;//TODO: remove

   friend class Zone;

   friend class Heap;
   friend class NewZoneTest;
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
   uword fromspace_;
   uword tospace_;

   NewZone(const uword start_address, const word size, const word semi_size):
    Zone(start_address, size, flags::GetNewPageSize()),
    fromspace_(GetStartingAddress()),
    tospace_(GetStartingAddress() + semi_size),
    semisize_(semi_size) {
   }

   static inline word GetFromspaceOffset() {
     return 0;
   }

   inline word GetTospaceOffset() const {
     return GetSemispaceSize();
   }
  public:
   NewZone() = delete;
   explicit NewZone(const MemoryRegion& region, const word semi_size = flags::GetNewZoneSemispaceSize()):
    NewZone(region.GetStartingAddress(), region.GetSize(), semi_size) {
   }
   explicit NewZone(const word size, const word semi_size = flags::GetNewZoneSemispaceSize()):
    NewZone(MemoryRegion(size), semi_size) {
   }
   NewZone(const NewZone& rhs) = delete;
   ~NewZone() override = default;

   uword GetFromspaceStartingAddress() const {
     return GetStartingAddress() + GetFromspaceOffset();
   }

   uword fromspace() const {
     return fromspace_;
   }

   uword GetFromspaceEndingAddress() const {
     return GetFromspaceStartingAddress() + GetSemispaceSize();
   }

   Semispace GetFromspace() const {
     return Semispace(Space::kFromSpace, GetFromspaceStartingAddress(), fromspace(), GetSemispaceSize());
   }

   uword GetTospaceStartingAddress() const {
     return GetStartingAddress() + GetTospaceOffset();
   }

   uword tospace() const {
     return tospace_;
   }

   uword GetTospaceEndingAddress() const {
     return GetTospaceStartingAddress() + GetSemispaceSize();
   }

   Semispace GetTospace() const {
     return Semispace(Space::kToSpace, GetTospaceStartingAddress(), tospace(), GetSemispaceSize());
   }

   word GetSemispaceSize() const {
     return semisize_;
   }

   virtual void SwapSpaces();

   void Clear() override {
     Zone::Clear();
     fromspace_ = GetStartingAddress();
     tospace_ = GetStartingAddress() + GetSemispaceSize();
   }

   bool IsEmpty() const {
     return GetStartingAddress() == fromspace();
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
     stream << "fromspace=" << val.GetFromspace() << ", ";
     stream << "tospace=" << val.GetTospace()  << ", ";
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