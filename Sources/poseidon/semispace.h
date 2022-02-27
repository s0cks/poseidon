#ifndef POSEIDON_POSEIDON_SEMISPACE_H
#define POSEIDON_POSEIDON_SEMISPACE_H

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
   uint64_t size_;
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
   Semispace(uword start, uint64_t size):
     start_(start),
     current_(start),
     size_(size){
   }
   Semispace(const Semispace& rhs) = default;
   ~Semispace() = default;

   uword GetStartingAddress() const{
     return start_;
   }

   void* GetStartingAddressPointer() const{
     return (void*)GetStartingAddress();
   }

   uint64_t size() const{
     return size_;
   }

   uword GetCurrentAddress() const{
     return current_;
   }

   void* GetCurrentAddressPointer() const{
     return (void*)GetCurrentAddress();
   }

   uword GetEndingAddress() const{
     return GetStartingAddress() + size();
   }

   void* GetEndingAddressPointer() const{
     return (void*)GetEndingAddress();
   }

   bool Contains(const uword& addr) const{
     return GetStartingAddress() <= addr
         && GetEndingAddress() >= addr;
   }

   void VisitRawObjects(RawObjectPointerVisitor* vis) const{
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

   void VisitMarkedRawObjects(RawObjectPointerVisitor* vis) const{
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

   bool IsEmpty() const{
     return GetStartingAddress() == GetCurrentAddress();
   }

   void Clear(){
     memset(GetStartingAddressPointer(), 0, size());
     current_ = start_;
   }

   uint64_t GetNumberOfBytesAllocated() const{
     return GetCurrentAddress() - GetStartingAddress();
   }

   uint64_t GetNumberOfBytesRemaining() const{
     return size() - GetNumberOfBytesAllocated();
   }

   RawObject* AllocateRawObject(const uint64_t& size);

   Semispace& operator=(const Semispace& rhs){
     if(this == &rhs)
       return *this;
     start_ = rhs.start_;
     current_ = rhs.current_;
     size_ = rhs.size_;
     return *this;
   }

   friend std::ostream& operator<<(std::ostream& stream, const Semispace& space){
     stream << "Semispace(";
     stream << "start=" << ((void*)space.GetStartingAddress()) << ", ";
     stream << "allocated=" << HumanReadableSize(space.GetNumberOfBytesAllocated()) << " (" << PrettyPrintPercentage(space.GetNumberOfBytesAllocated(), space.size()) << "), ";
     stream << "total_size=" << HumanReadableSize(space.size());
     stream << ")";
     return stream;
   }
 };
}

#endif//POSEIDON_POSEIDON_SEMISPACE_H