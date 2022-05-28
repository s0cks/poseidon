#ifndef POSEIDON_HEAP_SEMISPACE_H
#define POSEIDON_HEAP_SEMISPACE_H

#include <ostream>
#include <glog/logging.h>

#include "poseidon/raw_object.h"
#include "poseidon/heap/section.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon{
 class Semispace : public Section{
  private:
   class SemispaceIterator : public RawObjectPointerIterator{
    private:
     const Semispace* semispace_;
     uword current_;

     inline uword
     current_address() const{
       return current_;
     }

     inline RawObject*
     current_ptr() const{
       return (RawObject*)current_;
     }

     inline uword
     next_address() const{
       return current_ + current_ptr()->GetTotalSize();
     }

     inline uword
     GetStartingAddress() const{
       return semispace()->GetStartingAddress();
     }

     inline uword
     GetEndingAddress() const{
       return semispace()->GetEndingAddress();
     }

     inline uword
     GetCurrentAddress() const{
       return semispace()->GetCurrentAddress();
     }
    public:
     explicit SemispaceIterator(const Semispace* semispace):
       semispace_(semispace),
       current_(GetStartingAddress()){
     }
     ~SemispaceIterator() override = default;

     const Semispace* semispace() const{
       return semispace_;
     }

     bool HasNext() const override{
       return current_address() < GetCurrentAddress();
     }

     RawObject* Next() override{
       auto next = current_ptr();
       current_ += next->GetTotalSize();
       return next;
     }
   };
  private:
   RelaxedAtomic<uword> current_;
  public:
   Semispace():
    Section(),
    current_(0){
   }

   /**
    * Create a {@link Semispace} with the specified starting address and size.
    *
    * @param start The starting address for the {@link Semispace}
    * @param size The size of the {@link Semispace}
    */
   Semispace(uword start, int64_t size):
    Section(start, size),
    current_(start){
   }

   Semispace(MemoryRegion* region, int64_t offset, int64_t size):
    Semispace(region->GetStartingAddress() + offset, size){
   }

   Semispace(MemoryRegion* region, int64_t size):
    Semispace(region, 0, size){
   }

   explicit Semispace(MemoryRegion* region):
    Semispace(region, region->size()){
   }

   Semispace(const Semispace& rhs) = default;
   ~Semispace() override = default;

   uword GetCurrentAddress() const{
     return (uword)current_;
   }

   void* GetCurrentAddressPointer() const{
     return (void*)GetCurrentAddress();
   }

   bool IsEmpty() const{
     return GetStartingAddress() == GetCurrentAddress();
   }

   bool IsFull() const{
     return GetCurrentAddress() == GetEndingAddress();
   }

   void Clear(){
     memset(GetStartingAddressPointer(), 0, GetSize());
     current_ = start_;
   }

   uword TryAllocate(int64_t size);

   void VisitPointers(RawObjectVisitor* vis) const{
     return IteratePointers<Semispace, SemispaceIterator>(this, vis);
   }

   void VisitPointers(RawObjectVisitorFunction vis) const{
     return IteratePointers<Semispace, SemispaceIterator>(this, vis);
   }

   void VisitMarkedObjects(RawObjectVisitor* vis) const{
     return IterateMarkedPointers<Semispace, SemispaceIterator>(this, vis);
   }

   void VisitMarkedObjects(RawObjectVisitorFunction vis) const{
     return IterateMarkedPointers<Semispace, SemispaceIterator>(this, vis);
   }

   int64_t GetNumberOfBytesAllocated() const{
     return static_cast<int64_t>(GetCurrentAddress() - GetStartingAddress());
   }

   int64_t GetNumberOfBytesRemaining() const{
     return GetSize() - GetNumberOfBytesAllocated();
   }

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