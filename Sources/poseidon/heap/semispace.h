#ifndef POSEIDON_HEAP_SEMISPACE_H
#define POSEIDON_HEAP_SEMISPACE_H

#include <ostream>
#include <glog/logging.h>

#include "poseidon/raw_object.h"
#include "poseidon/heap/section.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon{
 class Semispace : public AllocationSection {
   friend class SemispaceTest;

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

     inline RawObject* current_ptr() const {
       return (RawObject*)current_address();
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

     RawObject* Next() override {
       auto next = current_ptr();
       current_ += next->GetSize();
       return next;
     }
   };
  protected:
   MemoryRegion region_;
   RelaxedAtomic<uword> current_;

   uword TryAllocate(ObjectSize size) override;
   void Clear() override;
  public:
   Semispace():
    AllocationSection(),
    region_(),
    current_(0){
   }
   explicit Semispace(const MemoryRegion& region):
    AllocationSection(),
    region_(region),
    current_(region.GetStartingAddress()) {
     if(!region.Protect(MemoryRegion::kReadWrite)) {
       LOG(FATAL) << "failed to protect " << region;
     }
   }
   Semispace(const Semispace& rhs) = default;
   ~Semispace() override = default;

   uword GetAllocatableSize() const {
     return GetSize();
   }

   uword GetStartingAddress() const override {
     return region_.GetStartingAddress();
   }

   uword GetCurrentAddress() const override {
     return (uword)current_;
   }

   int64_t GetSize() const override {
     return region_.GetSize();
   }

   bool VisitPointers(RawObjectVisitor* vis) override;
   bool VisitMarkedPointers(RawObjectVisitor* vis) override;

   Semispace& operator=(const Semispace& rhs){
     if(this == &rhs)
       return *this;
     Section::operator=(rhs);
     current_ = rhs.GetCurrentAddress();
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

   friend std::ostream& operator<<(std::ostream& stream, const Semispace& space){
     stream << "Semispace(";
     stream << "start=" << ((void*)space.GetStartingAddress()) << ", ";
     stream << "allocated=" << Bytes(space.GetNumberOfBytesAllocated()) << " (" << PrettyPrintPercentage(space.GetNumberOfBytesAllocated(), space.GetSize()) << "), ";
     stream << "total_size=" << Bytes(space.GetSize());
     stream << ")";
     return stream;
   }
 };
}

#endif//POSEIDON_HEAP_SEMISPACE_H