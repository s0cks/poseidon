#ifndef POSEIDON_HEAP_SECTION_H
#define POSEIDON_HEAP_SECTION_H

#include "poseidon/raw_object.h"
#include "poseidon/platform/platform.h"

namespace poseidon{
 // Represents a section of memory
 class Section{
  protected:
   template<class S, class I>
   static inline void
   IteratePointers(const S* section, RawObjectVisitor* vis){
     I iter(section);
     while(iter.HasNext()){
       auto next = iter.Next();
       DLOG(INFO) << "next: " << next->ToString();
       if(!vis->Visit(next))
         return;
     }
   }

   template<class S, class I>
   static inline void
   IterateMarkedPointers(const S* section, RawObjectVisitor* vis){
     I iter(section);
     while(iter.HasNext()){
       auto next = iter.Next();
       if(next->IsMarked() && !vis->Visit(next))
         return;
     }
   }

   template<class S, class I>
   static inline void
   IteratePointers(const S* section, RawObjectVisitorFunction vis){
     I iter(section);
     while(iter.HasNext()){
       auto next = iter.Next();
       if(!vis(next))
         return;
     }
   }

   template<class S, class I>
   static inline void
   IterateMarkedPointers(const S* section, RawObjectVisitorFunction vis){
     I iter(section);
     while(iter.HasNext()){
       auto next = iter.Next();
       if(next->IsMarked() && !vis(next))
         return;
     }
   }
  protected:
   uword start_;
   int64_t size_;

   Section():
    start_(0),
    size_(0){
   }

   /**
    * Create a {@link Section} of {@param size} bytes, starting at {@param start}.
    *
    * @param start The starting address of this {@link Section}
    * @param size The size of this {@link Section}
    */
   Section(uword start, int64_t size):
    start_(start),
    size_(size){
   }
  public:
   Section(const Section& rhs):
    start_(rhs.GetStartingAddress()),
    size_(rhs.GetSize()){
   }
   virtual ~Section() = default;

   uword GetStartingAddress() const{
     return start_;
   }

   void* GetStartingAddressPointer() const{
     return (void*)GetStartingAddress();
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

   /**
    * Determines if the {@param address} is contained in this {@link Section}.
    *
    * @param address The address to check
    * @return True if the address is inside of this {@link Section}; False otherwise.
    */
   bool Contains(uword address) const{
     return GetStartingAddress() <= address
         && GetEndingAddress() >= address;
   }

   friend bool operator==(const Section& lhs, const Section& rhs){
     return lhs.GetStartingAddress() == rhs.GetStartingAddress()
         && lhs.GetSize() == rhs.GetSize();
   }

   friend bool operator!=(const Section& lhs, const Section& rhs){
     return !operator==(lhs, rhs);
   }
 };

 class AllocationSection : public Section{
   friend class RawObject;
  public:
   class Iterator : public RawObjectPointerIterator{
    protected:
     const AllocationSection* section_;
     uword current_;

     inline uword current_address() const{
       return current_;
     }

     inline RawObject* current_ptr() const{
       return (RawObject*)current_address();
     }

     inline uword next_address() const{
       return current_address() + current_ptr()->GetTotalSize();
     }

     inline RawObject* next_ptr() const{
       return (RawObject*)next_address();
     }

     inline const AllocationSection* section() const{
       return section_;
     }
    public:
     explicit Iterator(const AllocationSection* section):
      section_(section),
      current_(section->GetStartingAddress()){
     }
     ~Iterator() override = default;

     bool HasNext() const override{
       return current_address() < section()->GetEndingAddress()
           && current_ptr()->GetPointerSize() > 0;
     }

     RawObject* Next() override{
       auto next = current_ptr();
       current_ += next->GetTotalSize();
       return next;
     }
   };
  protected:
   uword current_;

   AllocationSection():
    Section(),
    current_(0){
   }

   AllocationSection(uword start, int64_t size):
    Section(start, size),
    current_(start){
   }
  public:
   AllocationSection(const AllocationSection& rhs):
    Section(rhs),
    current_(rhs.GetCurrentAddress()){
   }
   ~AllocationSection() override = default;

   uword GetCurrentAddress() const{
     return current_;
   }

   void* GetCurrentAddressPointer() const{
     return (void*)GetCurrentAddress();
   }

   virtual uword TryAllocate(int64_t size) = 0;
   virtual void VisitPointers(RawObjectVisitor* vis) const;
   virtual void VisitPointers(const std::function<bool(RawObject*)>& vis) const;
   virtual void VisitMarkedPointers(RawObjectVisitor* vis) const;
   virtual void VisitMarkedPointers(const std::function<bool(RawObject*)>& vis) const;

   int64_t GetNumberOfBytesAllocated() const{
     return static_cast<int64_t>(GetCurrentAddress() - GetStartingAddress());
   }

   double GetAllocatedPercentage() const{
     return GetPercentageOf(GetNumberOfBytesAllocated(), GetSize());
   }

   int64_t GetNumberOfBytesRemaining() const{
     return static_cast<int64_t>(GetSize() - GetNumberOfBytesAllocated());
   }

   double GetRemainingPercentage() const{
     return GetPercentageOf(GetNumberOfBytesRemaining(), GetSize());
   }

   AllocationSection& operator=(const AllocationSection& rhs){
     if(*this == rhs)
       return *this;
     Section::operator=(rhs);
     current_ = rhs.GetCurrentAddress();
     return *this;
   }

   friend bool operator==(const AllocationSection& lhs, const AllocationSection& rhs){
     return lhs.GetStartingAddress() == rhs.GetStartingAddress()
         && lhs.GetSize() == rhs.GetSize()
         && lhs.GetCurrentAddress() == rhs.GetCurrentAddress();
   }

   friend bool operator!=(const AllocationSection& lhs, const AllocationSection& rhs){
     return !operator==(lhs, rhs);
   }
 };
}

#endif//POSEIDON_HEAP_SECTION_H