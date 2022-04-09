#ifndef POSEIDON_POSEIDON_SEMISPACE_H
#define POSEIDON_POSEIDON_SEMISPACE_H

#include <glog/logging.h>

#include "poseidon/raw_object.h"

namespace poseidon{
 class Semispace{
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

     inline RawObject*
     next_ptr() const{
       return (RawObject*)next_address();
     }

     inline uword
     GetStartingAddress() const{
       return semispace()->GetStartingAddress();
     }

     inline uword
     GetEndingAddress() const{
       return semispace()->GetEndingAddress();
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
       PSDN_ASSERT(valid());
       if(next_address() > GetEndingAddress())
         return false;
       return next_ptr()->GetPointerSize() > 0;
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
     start_(start),
     current_(start),
     size_(size){
   }

   /**
    * Copy-Constructor.
    *
    * @param rhs The {@link Semispace} to copy
    */
   Semispace(const Semispace& rhs) = default;
   virtual ~Semispace() = default;

   uword GetStartingAddress() const{
     return start_;
   }

   void* GetStartingAddressPointer() const{
     return (void*)GetStartingAddress();
   }

   uword GetCurrentAddress() const{
     return current_;
   }

   void* GetCurrentAddressPointer() const{
     return (void*)GetCurrentAddress();
   }

   int64_t GetSize() const{
     return size_;
   }

   uword GetEndingAddress() const{
     return GetStartingAddress() + GetSize();
   }

   void* GetEndingAddressPointer() const{
     return (void*)GetEndingAddress();
   }

   bool IsEmpty() const{
     return GetStartingAddress() == GetCurrentAddress();
   }

   bool Contains(uword address) const{
     return GetStartingAddress() <= address
         && GetEndingAddress() <= address;
   }

   void Clear(){
     memset(GetStartingAddressPointer(), 0, GetSize());
     current_ = start_;
   }

   uword TryAllocate(int64_t size){
     auto total_size = static_cast<int64_t>(sizeof(RawObject) + size);
     if((current_ + total_size) > GetEndingAddress()){
       DLOG(ERROR) << "cannot allocate object of " << Bytes(total_size) << " in Semispace.";
       return 0;
     }

     auto address = current_;
     current_ += total_size;
     return address;
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
     return GetSize() - GetNumberOfBytesAllocated();
   }

   Semispace& operator=(const Semispace& rhs)= default;

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
     stream << "allocated=" << Bytes(space.GetNumberOfBytesAllocated()) << " (" << PrettyPrintPercentage(space.GetNumberOfBytesAllocated(), space.GetSize()) << "), ";
     stream << "total_size=" << Bytes(space.GetSize());
     stream << ")";
     return stream;
   }
 };
}

#endif//POSEIDON_POSEIDON_SEMISPACE_H