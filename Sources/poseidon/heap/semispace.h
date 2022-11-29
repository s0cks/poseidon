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
   uword start_;
   int64_t size_;
   RelaxedAtomic<uword> current_;

   void Clear() override;

   static inline constexpr ObjectSize
   GetTotalSizeNeededFor(const ObjectSize size) {
     return size + static_cast<ObjectSize>(sizeof(Pointer));
   }
  public:
   Semispace():
    AllocationSection(),
    start_(0),
    current_(0),
    size_(0) {
   }
   explicit Semispace(const uword start, const int64_t size):
    AllocationSection(),
    start_(start),
    current_(start),
    size_(size) {
   }
   explicit Semispace(const MemoryRegion& region):
    Semispace(region.GetStartingAddress(), region.GetSize()) {
   }
   explicit Semispace(const int64_t size):
    Semispace(MemoryRegion(size)) {
   }
   Semispace(const Semispace& rhs) = default;
   ~Semispace() override = default;

   uword GetStartingAddress() const override {
     return start_;
   }

   uword GetCurrentAddress() const override {
     return (uword)current_;
   }

   word GetSize() const override {
     return size_;
   }

   uword TryAllocateBytes(ObjectSize size);
   uword TryAllocateClassBytes(Class* cls);

   uword TryAllocate(ObjectSize size) override {
     return TryAllocateBytes(size); //TODO: remove
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
     start_ = rhs.GetStartingAddress();
     current_ = rhs.GetCurrentAddress();
     size_ = rhs.GetSize();
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
}

#endif//POSEIDON_HEAP_SEMISPACE_H