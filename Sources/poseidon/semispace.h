#ifndef POSEIDON_POSEIDON_SEMISPACE_H
#define POSEIDON_POSEIDON_SEMISPACE_H

#include <glog/logging.h>
#include "poseidon/raw_object.h"

namespace poseidon{
 class Semispace : public AllocationSection{
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
     GetStartingAddress() const{
       return semispace()->GetStartingAddress();
     }

     inline uword
     GetEndingAddress() const{
       return semispace()->GetEndingAddress();
     }

     inline RawObject*
     GetNextPointer() const{
       return (RawObject*)(current_address() + current_ptr()->GetTotalSize());
     }

#ifdef PSDN_DEBUG
     inline bool
     valid() const{
       return GetStartingAddress() <= current_address()
           && current_address() <= GetEndingAddress();
     }
#endif//PSDN_DEBUG
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
#ifdef PSDN_DEBUG
       assert(valid());
#endif//PSDN_DEBUG
       return (current_address() + current_ptr()->GetTotalSize()) < GetEndingAddress()
           && GetNextPointer()->GetTotalSize() > 0;//TODO: make validity check
     }

     RawObject* Next() override{
       auto next = current_ptr();
       current_ += next->GetTotalSize();
       return next;
     }
   };
  private:
   uword start_;
   uword current_;
   int64_t size_;
  public:
   /**
    * Create an empty {@link Semispace}.
    */
   Semispace():
    AllocationSection(),
    start_(0),
    current_(0),
    size_(0){
   }
   /**
    * Create a {@link Semispace} with the specified starting address and size.
    *
    * @param start The starting address for the {@link Semispace}
    * @param size The size of the {@link Semispace}
    */
   Semispace(uword start, int64_t size):
    AllocationSection(),
    start_(start),
    current_(start),
    size_(size){
   }
   Semispace(const Semispace& rhs):
    AllocationSection(),
    start_(rhs.start_),
    current_(rhs.current_),
    size_(rhs.size_){
   }
   ~Semispace() override = default;

   uword GetStartingAddress() const override{
     return start_;
   }

   int64_t size() const override{
     return size_;
   }

   uword GetCurrentAddress() const{
     return current_;
   }

   void* GetCurrentAddressPointer() const{
     return (void*)GetCurrentAddress();
   }

   uword GetEndingAddress() const override{
     return GetStartingAddress() + size();
   }

   bool IsEmpty() const{
     return GetStartingAddress() == GetCurrentAddress();
   }

   void Clear() override{
     AllocationSection::Clear();
     current_ = start_;
   }

   uword Allocate(int64_t size) override{
     auto total_size = static_cast<int64_t>(sizeof(RawObject)) + size;
     if(!Contains(current_ + total_size)){
       DLOG(WARNING) << "cannot allocate object of size " << Bytes(size) << " in space.";
       return 0;
     }

     auto next = (void*)current_;
     current_ += total_size;
     auto ptr = new (next)RawObject();
     ptr->SetPointerSize(size);
     return ptr->GetAddress();
   }

   void VisitRawObjects(RawObjectVisitor* vis) const{
     SemispaceIterator iter(this);
     while(iter.HasNext()){
       if(!vis->Visit(iter.Next()))
         return;
     }
   }

   void VisitRawObjects(const std::function<bool(RawObject*)>& vis) const{
     SemispaceIterator iter(this);
     while(iter.HasNext()){
       if(!vis(iter.Next()))
         return;
     }
   }

   void VisitMarkedRawObjects(RawObjectVisitor* vis) const{
     SemispaceIterator iter(this);
     while(iter.HasNext()){
       if(!vis->Visit(iter.Next()))
         return;
     }
   }

   void VisitMarkedRawObjects(const std::function<bool(RawObject*)>& vis) const{
     SemispaceIterator iter(this);
     while(iter.HasNext()){
       auto next = iter.Next();
       if(next->IsMarked() && !vis(next))
         return;
     }
   }

   int64_t GetNumberOfBytesAllocated() const{
     return static_cast<int64_t>(GetCurrentAddress() - GetStartingAddress());
   }

   int64_t GetNumberOfBytesRemaining() const{
     return size() - GetNumberOfBytesAllocated();
   }

   Semispace& operator=(const Semispace& rhs){
     if(this == &rhs)
       return *this;
     start_ = rhs.start_;
     current_ = rhs.current_;
     size_ = rhs.size_;
     return *this;
   }

   friend bool operator==(const Semispace& lhs, const Semispace& rhs){
     return lhs.size_ == rhs.size_
         && lhs.start_ == rhs.start_
         && lhs.current_ == rhs.current_;
   }

   friend bool operator!=(const Semispace& lhs, const Semispace& rhs){
     return lhs.size_ != rhs.size_
         || lhs.start_ != rhs.start_
         || lhs.current_ != rhs.current_;
   }

   friend std::ostream& operator<<(std::ostream& stream, const Semispace& space){
     stream << "Semispace(";
     stream << "start=" << ((void*)space.GetStartingAddress()) << ", ";
     stream << "allocated=" << Bytes(space.GetNumberOfBytesAllocated()) << " (" << PrettyPrintPercentage(space.GetNumberOfBytesAllocated(), space.size()) << "), ";
     stream << "total_size=" << Bytes(space.size());
     stream << ")";
     return stream;
   }
 };
}

#endif//POSEIDON_POSEIDON_SEMISPACE_H