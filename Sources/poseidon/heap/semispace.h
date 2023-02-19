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

   static inline constexpr word
   GetTotalSizeNeededFor(const word size) {
     return size + static_cast<word>(sizeof(Pointer));
   }
  public:
   Semispace():
    AllocationSection(),
    space_(Space::kUnknownSpace) {
   }
   explicit Semispace(const Space space, const uword start, const uword current, const word size):
    AllocationSection(start, current, size),
    space_(space) { }
   explicit Semispace(const Space space, const uword start, const word size):
    AllocationSection(start, size),
    space_(space) {
   }
   explicit Semispace(const Space space, const MemoryRegion& region):
    Semispace(space, region.GetStartingAddress(), region.GetSize()) {
   }
   explicit Semispace(const MemoryRegion& region):
    Semispace(Space::kUnknownSpace, region) {
   }
   explicit Semispace(const Space space, const word size):
    Semispace(space, MemoryRegion(size)) {
   }
   explicit Semispace(const word size):
    Semispace(Space::kUnknownSpace, size) {
   }
   Semispace(const Semispace& rhs):
    AllocationSection(rhs),
    space_(rhs.space()) {
   }
   ~Semispace() override = default;

   Space space() const {
     return space_;
   }

   bool IsFromspace() const {
     return space() == Space::kFromSpace;
   }

   bool IsTospace() const {
     return space() == Space::kToSpace;
   }

   Pointer* TryAllocatePointer(word size);
   uword TryAllocateBytes(word size);
   uword TryAllocateClassBytes(Class* cls);

   template<typename T>
   T* TryAllocate() {
     return (T*) TryAllocateBytes(sizeof(T));
   }

   template<class T>
   T* TryAllocateClassBytes() {
     return TryAllocateClassBytes(T::GetClass());
   }

   bool VisitPointers(RawObjectVisitor* vis) override {
     return IteratePointers<Semispace, SemispaceIterator>(vis);
   }

   bool VisitUnmarkedPointers(RawObjectVisitor* vis) override {
     return IterateUnmarkedPointers<Semispace, SemispaceIterator>(vis);
   }

   bool VisitMarkedPointers(RawObjectVisitor* vis) override {
     return IterateMarkedPointers<Semispace, SemispaceIterator>(vis);
   }

   bool VisitNewPointers(RawObjectVisitor* vis) override {
     NOT_IMPLEMENTED(ERROR); //TODO: implement
     return false;
   }

   bool VisitOldPointers(RawObjectVisitor* vis) override {
     return false; // does not compute
   }

   Semispace& operator=(const Semispace& rhs){
     if(this == &rhs)
       return *this;
     AllocationSection::operator=(rhs);
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
 };

 class SemispacePrinter : public SectionPrinter<Semispace> {
  protected:
   explicit SemispacePrinter(const google::LogSeverity severity):
     SectionPrinter<Semispace>(severity) {
   }

   bool PrintSection(Semispace* semispace) override {
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
     return SectionPrinter<Semispace>::PrintSection(semispace);
   }
  public:
   ~SemispacePrinter() override = default;
  public:
   template<const google::LogSeverity Severity = google::INFO>
   static inline bool
   Print(Semispace* semispace) {
     SemispacePrinter printer(Severity);
     return printer.PrintSection(semispace);
   }
 };
}

#endif//POSEIDON_HEAP_SEMISPACE_H