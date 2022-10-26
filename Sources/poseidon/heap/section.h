#ifndef POSEIDON_HEAP_SECTION_H
#define POSEIDON_HEAP_SECTION_H

#include "poseidon/raw_object.h"
#include "poseidon/platform/platform.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon{
 class Section : public Region {
  protected:
   Section() = default;
  public:
   ~Section() override = default;
   virtual bool VisitPointers(RawObjectVisitor* vis) = 0;
   virtual bool VisitMarkedPointers(RawObjectVisitor* vis) = 0;

   friend bool operator==(const Section& lhs, const Section& rhs) {
     return ((const Region&)lhs) == ((const Region&)rhs);
   }
 };

 class AllocationSection : public Section {
  protected:
   AllocationSection() = default;

   virtual uword TryAllocate(ObjectSize size) = 0;
   virtual void Clear() = 0;
  public:
   ~AllocationSection() override = default;
   virtual uword GetCurrentAddress() const = 0;

   virtual bool IsEmpty() const {
     return GetCurrentAddress() == GetStartingAddress();
   }

   virtual bool IsFull() const {
     return GetCurrentAddress() == GetEndingAddress();
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

   virtual void* GetCurrentAddressPointer() const {
     return (void*) GetCurrentAddress();
   }

   friend bool operator==(const AllocationSection& lhs, const AllocationSection& rhs) {
     return ((const Section&)lhs) == ((const Section&)rhs) &&
            lhs.GetCurrentAddress() == rhs.GetCurrentAddress();
   }
 };
}

#endif//POSEIDON_HEAP_SECTION_H