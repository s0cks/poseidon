#ifndef POSEIDON_HEAP_SECTION_H
#define POSEIDON_HEAP_SECTION_H

#include "poseidon/pointer.h"
#include "poseidon/platform/platform.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon{
 class Section : public Region {
  protected:
   Section() = default;

   void Protect(MemoryRegion::ProtectionMode mode);
  public:
   ~Section() override = default;
   virtual bool VisitPointers(RawObjectVisitor* vis) = 0;
   virtual bool VisitMarkedPointers(RawObjectVisitor* vis) = 0;

   inline void SetReadOnly() { //TODO: visible for testing
     return Protect(MemoryRegion::kReadOnly);
   }

   inline void SetWritable() { //TODO: visible for testing
     return Protect(MemoryRegion::kReadWrite);
   }

   virtual void Clear() {
     memset(GetStartingAddressPointer(), 0, GetSize());
   }

   friend std::ostream& operator<<(std::ostream& stream, const Section& rhs) {
     stream << "Section(";
     stream << "start=" << rhs.GetStartingAddress() << ", ";
     stream << "size=" << rhs.GetSize();
     stream << ")";
     return stream;
   }
 };

 class AllocationSection : public Section {
  protected:
   AllocationSection() = default;

   virtual uword TryAllocate(ObjectSize size) = 0;
  public:
   ~AllocationSection() override = default;

   virtual uword GetCurrentAddress() const = 0;

   virtual void* GetCurrentAddressPointer() const {
     return (void*) GetCurrentAddress();
   }

   virtual bool IsEmpty() const {
     return GetCurrentAddress() == GetStartingAddress();
   }

   virtual bool IsFull() const {
     return GetCurrentAddress() == GetEndingAddress();
   }

   virtual int64_t GetAllocatableSize() const {
     return GetSize();
   }

   virtual int64_t GetNumberOfBytesAllocated() const{
     return static_cast<int64_t>(GetCurrentAddress() - GetStartingAddress());
   }

   virtual double GetPercentageBytesAllocated() const {
     return GetPercentageOf(GetNumberOfBytesAllocated(), GetSize());
   }

   virtual int64_t GetNumberOfBytesRemaining() const{
     return GetSize() - GetNumberOfBytesAllocated();
   }

   virtual double GetPercentageBytesRemaining() const {
     return GetPercentageOf(GetNumberOfBytesRemaining(), GetSize());
   }
 };
}

#endif//POSEIDON_HEAP_SECTION_H