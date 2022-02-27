#ifndef POSEIDON_POSEIDON_ZONE_H
#define POSEIDON_POSEIDON_ZONE_H

#include "poseidon/semispace.h"
#include "poseidon/memory_region.h"

namespace poseidon{
 class Zone{
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
   uint64_t size_;
   Semispace from_;
   Semispace to_;

   static inline uint64_t
   GetSemispaceSize(uint64_t zone_size){
     return zone_size / 2;
   }
  public:
   /**
    * Create an empty {@link Zone}.
    */
   Zone():
    start_(0),
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
   Zone(const MemoryRegion& region, uint64_t offset, uint64_t size)://TODO: refactor?
    Zone(region.GetStartAddress() + offset, size){
   }

   /**
    * Create a {@link Zone} with the specified starting address and size.
    *
    * @param start The starting address for the {@link Zone}
    * @param size The size of the {@link Zone}
    */
   Zone(uword start, uint64_t size)://TODO: cleanup?
    start_(start),
    size_(size),
    from_(start, GetSemispaceSize(size)),
    to_(start + GetSemispaceSize(size), GetSemispaceSize(size)){
   }
   Zone(const Zone& rhs) = default; // Copy-Constructor
   ~Zone() = default; // Destructor

   Semispace from() const{
     return from_;
   }

   Semispace to() const{
     return to_;
   }

   /**
    * Returns the address of the beginning of this {@link Zone}.
    *
    * @return The address of the beginning of this {@link Zone}.
    */
   uword GetStartingAddress() const{
     return start_;
   }

   /**
    * Returns a pointer to the beginning of this {@link Zone}.
    *
    * @return A pointer to the beginning of this {@link Zone}
    */
   void* GetStartingAddressPointer() const{
     return (void*)GetStartingAddress();
   }

   uint64_t size() const{
     return size_;
   }

   /**
    * Returns the address at the end of this {@link Zone}.
    *
    * @return The address of the end of this {@link Zone}.
    */
   uword GetEndingAddress() const{
     return GetStartingAddress() + size();
   }

   /**
    * Returns a pointer to the end of this {@link Zone}.
    *
    * @return A pointer to the end of this {@link Zone}.
    */
   void* GetEndingAddressPointer() const{
     return (void*)GetEndingAddress();
   }

   /**
    * Check whether or not the address is inside of this this {@link Zone} or not.
    *
    * @param address The address to check
    * @return True if the address is contained in this {@link Zone}
    */
   bool Contains(uword address) const{
     return GetStartingAddress() <= address
         && GetEndingAddress() >= address;
   }

   void VisitRawObjects(RawObjectPointerVisitor* vis) const{//TODO: document
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

   void VisitMarkedRawObjects(RawObjectPointerVisitor* vis) const{//TODO: document
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
    * Clears all the bytes in the {@link Zone}.
    */
   void ClearZone() const{
     memset(GetStartingAddressPointer(), 0, size());
   }

   /**
    * Swaps the from_ & to_ {@link Semispace}s in this {@link Zone}.
    *
    * Called during collection time.
    */
   void SwapSpaces(){
     std::swap(from_, to_);
   }

   /**
    * Allocates a new object of {@param size} bytes in the from_ {@link Semispace} of this {@link Zone}.
    *
    * @param size The size of the new object to allocate
    * @return A pointer to the beginning of the object and i's header
    */
   RawObject* AllocateRawObject(uint64_t size){//TODO: refactor/cleanup?
     return from_.AllocateRawObject(size);
   }

   Zone& operator=(const Zone& rhs){
     if(this == &rhs)
       return *this;
     start_ = rhs.GetStartingAddress();
     size_ = rhs.size();
     return *this;
   }

   friend std::ostream& operator<<(std::ostream& stream, const Zone& zone){
     stream << "Zone(";
     stream << "starting_address=" << zone.GetStartingAddressPointer() << ", ";
     stream << "total_size=" << HumanReadableSize(zone.size());
     stream << ")";
     return stream;
   }
 };
}

#endif //POSEIDON_POSEIDON_ZONE_H
