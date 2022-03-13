#ifndef POSEIDON_POSEIDON_ZONE_H
#define POSEIDON_POSEIDON_ZONE_H

#include "poseidon/semispace.h"
#include "poseidon/memory_region.h"

namespace poseidon{
 class Zone : public AllocationSection{
  private:
   class ZoneIterator : public RawObjectPointerIterator{
    private:
     const Zone* zone_;
     uword ptr_;

     /**
      * Returns The current {@link RawObject} pointer for this {@link RawObjectPointerIterator}.
      *
      * @return The current {@link RawObject} pointer for this {@link RawObjectPointerIterator}
      */
     RawObject* current_ptr() const{
       return (RawObject*)ptr_;
     }

     /**
      * Returns The next {@link RawObject} pointer for this {@link RawObjectPointerIterator}.
      *
      * @return The next {@link RawObject} pointer for this {@link RawObjectPointerIterator}
      */
     RawObject* next_ptr() const{
       return (RawObject*)ptr_ + current_ptr()->GetTotalSize();
     }
    public:
     /**
      * An {@link RawObjectPointerIterator} for a {@link Zone}.
      *
      * @param zone The {@link Zone} to iterate
      */
     explicit ZoneIterator(const Zone* zone):
      zone_(zone),
      ptr_(zone->GetStartingAddress()){
     }
     ~ZoneIterator() override = default; // Destructor

     const Zone* zone() const{
       return zone_;
     }

     /**
      * Check for whether or not this {@link RawObjectPointerIterator} has any more {@link RawObject} pointers to visit.
      *
      * @return True if there are more {@link RawObject} pointers to visit, false otherwise
      */
     bool HasNext() const override{
       auto next = next_ptr();
#ifdef PSDN_DEBUG
       assert(zone()->Contains(next->GetAddress()));
#endif//PSDN_DEBUG
       return next->GetPointerSize() > 0;
     }

     /**
      * Returns the next {@link RawObject} pointer in the {@link Zone}.
      *
      * @return The next {@link RawObject} pointer in the {@link Zone}
      */
     RawObject* Next() override{
       auto next = current_ptr();
       ptr_ += next->GetTotalSize();
       return next;
     }
   };
  private:
   uword start_;
   uword current_;
   int64_t size_;

   Semispace from_;
   Semispace to_;

   static inline int64_t
   GetSemispaceSize(int64_t zone_size){
     return zone_size / 2;
   }
  public:
   /**
    * Create an empty {@link Zone}.
    */
   Zone():
    start_(0),
    current_(0),
    size_(0),
    from_(),
    to_(){
   }

   /**
    * Create a {@link Zone} within the {@link MemoryRegion} at the specified offset and size.
    *
    * @param region The {@link MemoryRegion} to create the {@link Zone} in
    * @param offset The offset for the {@link Zone} in the {@link MemoryRegion}
    * @param size The size of the {@link Zone}
    */
   Zone(MemoryRegion* region, int64_t offset, int64_t size)://TODO: refactor?
    Zone(region->GetStartAddress() + offset, size){
   }

   /**
    * Create a {@link Zone} with the specified starting address and size.
    *
    * @param start The starting address for the {@link Zone}
    * @param size The size of the {@link Zone}
    */
   Zone(uword start, int64_t size)://TODO: cleanup?
    start_(start),
    current_(start),
    size_(size),
    from_(start, GetSemispaceSize(size)),
    to_(start + GetSemispaceSize(size), GetSemispaceSize(size)){
   }
   Zone(const Zone& rhs) = default; // Copy-Constructor
   ~Zone() override = default; // Destructor

   /**
    * Returns the address of the beginning of this {@link Zone}.
    *
    * @return The address of the beginning of this {@link Zone}.
    */
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

   void VisitRawObjects(RawObjectVisitor* vis) const{//TODO: document
     ZoneIterator iter(this);
     while(iter.HasNext()){
       if(!vis->Visit(iter.Next()))
         return;
     }
   }

   void VisitRawObjects(const std::function<bool(RawObject*)>& vis) const{//TODO: document
     ZoneIterator iter(this);
     while(iter.HasNext()){
       if(!vis(iter.Next()))
         return;
     }
   }

   void VisitMarkedRawObjects(RawObjectVisitor* vis) const{//TODO: document
     ZoneIterator iter(this);
     while(iter.HasNext()){
       if(!vis->Visit(iter.Next()))
         return;
     }
   }

   void VisitMarkedRawObjects(const std::function<bool(RawObject*)>& vis) const{//TODO: document
     ZoneIterator iter(this);
     while(iter.HasNext()){
       auto next = iter.Next();
       if(next->IsMarked() && !vis(next))
         return;
     }
   }

   /**
    * Swaps the from_ & to_ {@link Semispace}s in this {@link Zone}.
    *
    * Called during collection time.
    */
   virtual void SwapSpaces(){
     std::swap(from_, to_);
   }

   int64_t GetNumberOfBytesAllocated() const{
     return static_cast<int64_t>(GetCurrentAddress() - GetStartingAddress());
   }

   /**
    * Allocates a new object of {@param size} bytes in the from_ {@link Semispace} of this {@link Zone}.
    *
    * @param size The size of the new object to allocate
    * @return A pointer to the beginning of the object and i's header
    */
   uword Allocate(int64_t size) override{
     return from_.Allocate(size);
   }

   Zone& operator=(const Zone& rhs){
     if(this == &rhs)
       return *this;
     start_ = rhs.GetStartingAddress();
     size_ = rhs.size();
     from_ = rhs.from_;
     to_ = rhs.to_;
     return *this;
   }

   friend std::ostream& operator<<(std::ostream& stream, const Zone& zone){
     stream << "Zone(";
     stream << "starting_address=" << zone.GetStartingAddressPointer() << ", ";
     stream << "total_size=" << HumanReadableSize(zone.size()) << ", ";
     stream << "allocated=" << HumanReadableSize(zone.GetNumberOfBytesAllocated()) << " (" << PrettyPrintPercentage(zone.GetNumberOfBytesAllocated(), zone.size()) << ")";
     stream << ")";
     return stream;
   }
 };
}

#endif //POSEIDON_POSEIDON_ZONE_H
