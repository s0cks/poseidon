#ifndef POSEIDON_HEAP_SECTION_H
#define POSEIDON_HEAP_SECTION_H

#include "poseidon/pointer.h"
#include "poseidon/platform/platform.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon{
 class Section : public Region {
  protected:
   MemoryRegion region_;

   explicit Section(const uword start = 0, const RegionSize size = 0):
    Region(start, size),
    region_(start, size) {
     SetWritable();
   }
   explicit Section(const Region& region):
    Region(region) {
   }

   inline MemoryRegion& region() {
     return region_;
   }

   void Protect(MemoryRegion::ProtectionMode mode);

   template<class Z, class Iterator>
   inline bool
   IteratePointers(RawObjectVisitor* vis) {
     Iterator iter((Z*) this);
     while(iter.HasNext()) {
       auto next = iter.Next();
       if(!vis->Visit(next))
         return false;
     }
     return true;
   }

   template<class Z, class Iterator>
   inline bool IterateNewPointers(RawObjectVisitor* vis) {
     Iterator iter((Z*) this);
     while(iter.HasNext()) {
       auto next = iter.Next();
       if(next->IsNew() && !vis->Visit(next))
         return false;
     }
     return true;
   }

   template<class Z, class Iterator>
   inline bool IterateOldPointers(RawObjectVisitor* vis) {
     Iterator iter((Z*) this);
     while(iter.HasNext()) {
       auto next = iter.Next();
       if(next->IsOld() && !vis->Visit(next))
         return false;
     }
     return true;
   }

   template<class Z, class Iterator>
   inline bool
   IterateMarkedPointers(RawObjectVisitor* vis) {
     Iterator iter((Z*) this);
     while(iter.HasNext()) {
       auto next = iter.Next();
       if(next->IsMarked() && !vis->Visit(next))
         return false;
     }
     return true;
   }

   template<class Z, class Iterator>
   inline bool
   IterateUnmarkedPointers(RawObjectVisitor* vis) {
     Iterator iter((Z*) this);
     while(iter.HasNext()) {
       auto next = iter.Next();
       DLOG(INFO) << "next: " << (*next);
       if(next->IsMarked())
         continue;

       if(!vis->Visit(next))
         return false;
     }
     return true;
   }
  public: //TODO: visible for testing
   inline void SetNoAccess() {
     Protect(MemoryRegion::kNoAccess);
   }

   inline void SetReadOnly() {
     Protect(MemoryRegion::kReadOnly);
   }

   inline void SetWritable() {
     Protect(MemoryRegion::kReadWrite);
   }
  public:
   ~Section() override = default;

   virtual bool VisitPointers(RawObjectVisitor* vis) = 0;

   virtual bool VisitPointers(const RawObjectVisitor::VisitorFunction& function) {
     auto vis = RawObjectVisitorWrapper(function);
     return VisitPointers(&vis);
   }

   virtual bool VisitMarkedPointers(RawObjectVisitor* vis) = 0;

   virtual bool VisitMarkedPointers(const RawObjectVisitor::VisitorFunction& function) {
     auto vis = RawObjectVisitorWrapper(function);
     return VisitMarkedPointers(&vis);
   }

   virtual bool VisitUnmarkedPointers(RawObjectVisitor* vis) = 0;

   virtual bool VisitUnmarkedPointers(const RawObjectVisitor::VisitorFunction& func) {
     auto vis = RawObjectVisitorWrapper(func);
     return VisitUnmarkedPointers(&vis);
   }

   virtual bool VisitNewPointers(RawObjectVisitor* vis) = 0;

   virtual bool VisitNewPointers(const RawObjectVisitor::VisitorFunction& function) {
     auto vis = RawObjectVisitorWrapper(function);
     return VisitNewPointers(&vis);
   }

   virtual bool VisitOldPointers(RawObjectVisitor* vis) = 0;

   virtual bool VisitOldPointers(const RawObjectVisitor::VisitorFunction& function) {
     auto vis = RawObjectVisitorWrapper(function);
     return VisitOldPointers(&vis);
   }

   friend std::ostream& operator<<(std::ostream& stream, const Section& rhs) {
     stream << "Section(";
     stream << "start=" << rhs.GetStartingAddress() << ", ";
     stream << "size=" << rhs.GetSize();
     stream << ")";
     return stream;
   }

   Section& operator=(const Section& rhs) {
     if(&rhs == this)
       return *this;
     start_ = rhs.start_;
     size_ = rhs.size_;
     return *this;
   }
 };

 template<class S>
 class SectionPrinter : public RawObjectVisitor {
  protected:
   const google::LogSeverity severity_;

   explicit SectionPrinter(const google::LogSeverity severity):
     RawObjectVisitor(),
     severity_(severity) {
   }

   virtual bool PrintSection(S* section) {
     return section->VisitPointers(this);
   }
  public:
   ~SectionPrinter() override = default;

   google::LogSeverity GetSeverity() const {
     return severity_;
   }

   bool Visit(Pointer* raw_ptr) override {
     LOG_AT_LEVEL(GetSeverity()) << " - " << (*raw_ptr);
     return true;
   }
 };

 class AllocationSection : public Section {
  protected:
   uword current_;

   AllocationSection(const uword start, const word size):
    Section(start, size),
    current_(start) {
   }

   explicit AllocationSection(const Region& region):
    Section(region),
    current_(region.GetStartingAddress()) {
   }

   void Clear() override {
     Section::Clear();
     current_ = GetStartingAddress();
   }
  public:
   AllocationSection() = default;
   AllocationSection(const AllocationSection& rhs) = default;
   ~AllocationSection() override = default;

   virtual uword GetCurrentAddress() const {
     return current_;
   }

   virtual void* GetCurrentAddressPointer() const {
     return (void*) GetCurrentAddress();
   }

   virtual bool IsEmpty() const {
     return GetCurrentAddress() == GetStartingAddress();
   }

   virtual bool IsFull() const {
     return GetCurrentAddress() == GetEndingAddress();
   }

   virtual word GetAllocatableSize() const {
     return GetSize();
   }

   virtual word GetNumberOfBytesAllocated() const{
     return static_cast<int64_t>(GetCurrentAddress() - GetStartingAddress());
   }

   virtual double GetPercentageBytesAllocated() const {
     return GetPercentageOf(GetNumberOfBytesAllocated(), GetSize());
   }

   virtual word GetNumberOfBytesRemaining() const{
     return GetSize() - GetNumberOfBytesAllocated();
   }

   virtual double GetPercentageBytesRemaining() const {
     return GetPercentageOf(GetNumberOfBytesRemaining(), GetSize());
   }

   AllocationSection& operator=(const AllocationSection& rhs) {
     if(&rhs == this)
       return *this;
     Section::operator=(rhs);
     current_ = rhs.current_;
     return *this;
   }
 };
}

#endif//POSEIDON_HEAP_SECTION_H