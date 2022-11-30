#ifndef POSEIDON_HEAP_SEMISPACE_H
#define POSEIDON_HEAP_SEMISPACE_H

#include <ostream>
#include <glog/logging.h>

#include "poseidon/pointer.h"
#include "poseidon/heap/section.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon{
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
       return current_address() < semispace()->GetCurrentAddress();
     }

     Pointer* Next() override {
       auto next = current_ptr();
       current_ += next->GetTotalSize();
       return next;
     }
   };
  protected:
   void Clear() override;

   static inline constexpr ObjectSize
   GetTotalSizeNeededFor(const ObjectSize size) {
     return size + static_cast<ObjectSize>(sizeof(Pointer));
   }
  public:
   Semispace() = default;
   explicit Semispace(const uword start, const int64_t size):
    AllocationSection(start, size) {
     SetWritable();
   }
   explicit Semispace(const MemoryRegion& region):
    Semispace(region.GetStartingAddress(), region.GetSize()) {
   }
   explicit Semispace(const int64_t size):
    Semispace(MemoryRegion(size)) {
   }
   Semispace(const Semispace& rhs) = default;
   ~Semispace() override = default;

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

   bool VisitPointers(const std::function<bool(Pointer*)>& vis) override {
     return IteratePointers<Semispace, SemispaceIterator>(vis);
   }

   bool VisitMarkedPointers(RawObjectVisitor* vis) override {
     return IterateMarkedPointers<Semispace, SemispaceIterator>(vis);
   }

   bool VisitMarkedPointers(const std::function<bool(Pointer*)>& vis) override {
     return IterateMarkedPointers<Semispace, SemispaceIterator>(vis);
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
     stream << "size=" << Bytes(value.GetSize());
     stream << ")";
     return stream;
   }
  public:
   static inline constexpr ObjectSize
   GetMinimumObjectSize() { //TODO: refactor
     return kMinimumObjectSize;
   }

   static inline constexpr ObjectSize
   GetMaximumObjectSize() { //TODO: refactor
     return 1 * kMB;
   }
 };

 class SemispacePrinter : public RawObjectVisitor {
  protected:
   std::string name_;
   const google::LogSeverity severity_;
  public:
   explicit SemispacePrinter(std::string name, const google::LogSeverity severity):
    RawObjectVisitor(),
    name_(std::move(name)),
    severity_(severity) {
   }
   ~SemispacePrinter() override = default;

   std::string name() const {
     return name_;
   }

   google::LogSeverity GetSeverity() const {
     return severity_;
   }

   bool Visit(Pointer* raw_ptr) override {
     LOG_AT_LEVEL(GetSeverity()) << " - " << (*raw_ptr);
     return true;
   }

   bool PrintSemispace(Semispace* semispace) {
     LOG_AT_LEVEL(GetSeverity()) << name() << ":";
     return semispace->VisitPointers(this);
   }
  public:
   template<const google::LogSeverity Severity = google::INFO>
   static inline bool
   Print(const std::string& name, Semispace* semispace) {
     SemispacePrinter printer(name, Severity);
     return printer.PrintSemispace(semispace);
   }
 };
}

#endif//POSEIDON_HEAP_SEMISPACE_H