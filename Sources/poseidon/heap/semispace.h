#ifndef POSEIDON_HEAP_SEMISPACE_H
#define POSEIDON_HEAP_SEMISPACE_H

#include <ostream>
#include <glog/logging.h>

#include "poseidon/pointer.h"
#include "poseidon/heap/section.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon{
 enum class Space : uint8_t {
   kUnknownSpace = 0,
   kFromSpace = 1,
   kToSpace,
 };

 static inline std::ostream& operator<<(std::ostream& stream, const Space& value) {
   switch(value) {
     case Space::kFromSpace:
       return stream << "Fromspace";
     case Space::kToSpace:
       return stream << "Tospace";
     case Space::kUnknownSpace:
     default:
       return stream << "[unknown Space " << static_cast<word>(value) << "]";
   }
 }

 class Class;
 class Semispace : public AllocationSection {
   friend class SemispaceTest;

   friend class NewZone;
   friend class Scavenger;
  public:
   class SemispaceIterator : public RawObjectPointerIterator {
    protected:
     Semispace* semispace_;
     uword current_;

     inline Semispace* semispace() const {
       return semispace_;
     }

     inline uword current_address() const {
       return current_;
     }

     inline Pointer* current_ptr() const {
       return (Pointer*)current_address();
     }
    public:
     explicit SemispaceIterator(Semispace* semispace):
      RawObjectPointerIterator(),
      semispace_(semispace),
      current_(semispace->GetStartingAddress()) {
     }
     ~SemispaceIterator() override = default;

     bool HasNext() const override {
       return current_address() >= semispace()->GetStartingAddress() &&
              current_address() < semispace()->GetCurrentAddress() &&
              current_ptr()->IsNew() &&
              current_ptr()->GetSize() > 0;
     }

     Pointer* Next() override {
       auto next = current_ptr();
       current_ += next->GetTotalSize();
       return next;
     }
   };
  protected:
   Space space_;

   static inline constexpr ObjectSize
   GetTotalSizeNeededFor(const ObjectSize size) {
     return size + static_cast<ObjectSize>(sizeof(Pointer));
   }

   Pointer* TryAllocatePointer(ObjectSize size);
  public:
   Semispace():
    AllocationSection(),
    space_(Space::kUnknownSpace) {
   } //TODO: remove?
   Semispace(const Space space, const Semispace& semispace):
    AllocationSection(semispace.GetStartingAddress(), semispace.GetCurrentAddress(), semispace.GetSize()),
    space_(space) {
   }
   explicit Semispace(const Space space, const uword start, const uword current, const RegionSize size):
    AllocationSection(start, current, size),
    space_(space) { } //TODO: remove
   explicit Semispace(const Space space, const uword start, const RegionSize size):
    AllocationSection(start, size),
    space_(space) {
   }
   explicit Semispace(const Space space, const Region& region):
    Semispace(space, region.GetStartingAddress(), region.GetSize()) {
   }
   explicit Semispace(const Region& region):
    Semispace(Space::kUnknownSpace, region) {
   }
   explicit Semispace(const Space space, const RegionSize size):
    Semispace(space, MemoryRegion(size)) {
   }
   explicit Semispace(const RegionSize size):
    Semispace(Space::kUnknownSpace, size) {
   }
   Semispace(const Semispace& rhs):
    AllocationSection(rhs),
    space_(rhs.space()) {
   }
   ~Semispace() override = default;

   Space space() const { //TODO: is this necessary?
     return space_;
   }

   bool IsFromspace() const {
     return space() == Space::kFromSpace;
   }

   bool IsTospace() const {
     return space() == Space::kToSpace;
   }

   uword TryAllocateBytes(RegionSize size);
   uword TryAllocateClassBytes(Class* cls);

   template<typename T>
   T* TryAllocate() {
     return (T*) TryAllocateBytes(sizeof(T));
   }

   template<class T>
   T* TryAllocateClassBytes() {
     return TryAllocateClassBytes(T::GetClass());
   }

   bool VisitPointers(RawObjectVisitor* vis) const override {
     return IteratePointers<Semispace, SemispaceIterator>(vis);
   }

   bool VisitUnmarkedPointers(RawObjectVisitor* vis) const override {
     return IterateUnmarkedPointers<Semispace, SemispaceIterator>(vis);
   }

   bool VisitMarkedPointers(RawObjectVisitor* vis) const override {
     return IterateMarkedPointers<Semispace, SemispaceIterator>(vis);
   }

   bool VisitNewPointers(RawObjectVisitor* vis) const override {
     NOT_IMPLEMENTED(ERROR); //TODO: implement
     return false;
   }

   Semispace& operator=(const Semispace& rhs){
     if(this == &rhs)
       return *this;
     AllocationSection::operator=(rhs);
     space_ = rhs.space();
     return *this;
   }

   friend bool operator==(const Semispace& lhs, const Semispace& rhs){
     return lhs.GetStartingAddress() == rhs.GetStartingAddress()
         && lhs.GetSize() == rhs.GetSize()
         && lhs.GetCurrentAddress() == rhs.GetCurrentAddress();
   }

   friend bool operator!=(const Semispace& lhs, const Semispace& rhs){
     return !operator==(lhs, rhs);
   }

   friend std::ostream& operator<<(std::ostream& stream, const Semispace& value){
     stream << "Semispace(";
     stream << "space=" << value.space() << ", ";
     stream << "start=" << value.GetStartingAddressPointer()<< ", ";
     stream << "current=" << value.GetCurrentAddressPointer() << ", ";
     stream << "size=" << Bytes(value.GetSize()) << ", ";
     stream << "allocated=" << Bytes(value.GetNumberOfBytesAllocated()) << " " << PrettyPrintPercentage(value.GetPercentageBytesAllocated());
     stream << ")";
     return stream;
   }
  public:
   static inline constexpr word
   GetMinimumObjectSize() { //TODO: refactor
     return kMinimumObjectSize;
   }

   static inline constexpr word
   GetMaximumObjectSize() { //TODO: refactor
     return 1 * kMB;
   }

   static inline void
   Swap(Semispace& fromspace, Semispace& tospace) {
     Semispace tmp = fromspace;
     fromspace = Semispace(Space::kFromSpace, tospace);
     tospace = Semispace(Space::kToSpace, tmp);
   }
 };

 class SemispacePrinter : public SectionPrinter<Semispace> {
  protected:
   explicit SemispacePrinter(const google::LogSeverity severity):
     SectionPrinter<Semispace>(severity) {
   }

   bool PrintSection(const Semispace* semispace) {
     switch(semispace->space()) {
       case Space::kFromSpace:
         LOG_AT_LEVEL(GetSeverity()) << "Fromspace " << (*semispace) << ":";
         break;
       case Space::kToSpace:
         LOG_AT_LEVEL(GetSeverity()) << "Tospace " << (*semispace) << ":";
         break;
       case Space::kUnknownSpace:
       default:
         LOG_AT_LEVEL(GetSeverity()) << "<unknown space: " << static_cast<word>(semispace->space()) << "> " << (*semispace) << ":";
     }
     return semispace->VisitPointers(this);
   }
  public:
   ~SemispacePrinter() override = default;
  public:
   template<const google::LogSeverity Severity = google::INFO>
   static inline bool
   Print(const Semispace* semispace) {
     SemispacePrinter printer(Severity);
     return printer.PrintSection(semispace);
   }

   template<const google::LogSeverity Severity = google::INFO>
   static inline bool
   Print(const Semispace& semispace) {
     return Print(&semispace);
   }
 };
}

#endif//POSEIDON_HEAP_SEMISPACE_H