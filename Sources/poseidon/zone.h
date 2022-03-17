#ifndef POSEIDON_POSEIDON_ZONE_H
#define POSEIDON_POSEIDON_ZONE_H

#include "poseidon/semispace.h"
#include "poseidon/memory_region.h"

namespace poseidon{
 class NewPage;
 class Zone : public AllocationSection{
   friend class Scavenger;
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
  public:
   /**
    * Create an empty {@link Zone}.
    */
   Zone() = default;

   /**
    * Create a {@link Zone} with the specified starting address and size.
    *
    * @param start The starting address for the {@link Zone}
    * @param size The size of the {@link Zone}
    */
   Zone(uword start, int64_t size)://TODO: cleanup?
    start_(start),
    current_(start),
    size_(size){
   }

   /**
    * Create a {@link Zone} within the {@link MemoryRegion} at the specified offset and size.
    *
    * @param region The {@link MemoryRegion} to create the {@link Zone} in
    * @param offset The offset for the {@link Zone} in the {@link MemoryRegion}
    * @param size The size of the {@link Zone}
    */
   Zone(MemoryRegion* region, int64_t offset, int64_t size)://TODO: refactor?
       Zone(region->GetStartingAddress() + offset, size){
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
     stream << "total_size=" << Bytes(zone.size()) << ", ";
     stream << "allocated=" << Bytes(zone.GetNumberOfBytesAllocated()) << " (" << PrettyPrintPercentage(zone.GetNumberOfBytesAllocated(), zone.size()) << ")";
     stream << ")";
     return stream;
   }
 };

 class NewZone : public Zone{
  public:
   static inline int64_t
   CalculateSemispaceSize(int64_t zone_size){
     return zone_size / 2;
   }
  private:
   Semispace from_;
   Semispace to_;
  public:
   NewZone() = default;
   NewZone(uword start, int64_t size):
    Zone(start, size),
    from_(start, CalculateSemispaceSize(size)),
    to_(start + CalculateSemispaceSize(size), CalculateSemispaceSize(size)){
   }
   NewZone(MemoryRegion* region, int64_t offset, int64_t size):
     NewZone(region->GetStartingAddress() + offset, size){
   }
   ~NewZone() override = default;

   Semispace& from(){
     return from_;
   }

   Semispace& to(){
     return to_;
   }

  /**
   * Swaps the from_ & to_ {@link Semispace}s in this {@link Zone}.
   *
   * Called during collection time.
   */
   virtual void SwapSpaces(){
     std::swap(from_, to_);
   }

   void VisitPages(const std::function<bool(NewPage*)>& vis) const;

   NewZone& operator=(const NewZone& rhs){
     if(this == &rhs)
       return *this;
     Zone::operator=(rhs);
     from_ = rhs.from_;
     to_ = rhs.to_;
     return *this;
   }

   friend std::ostream& operator<<(std::ostream& stream, const NewZone& val){
     return stream << (Zone&)val;
   }
 };

 class OldZone : public Zone{
  public:
   class OldPage{
     friend class OldZone;
    public:
     class OldPageIterator : public RawObjectPointerIterator{
      private:
       const OldPage* page_;
       uword current_;

       inline RawObject* current_ptr() const{
         return (RawObject*)current_;
       }

       inline RawObject* next_ptr() const{
         return (RawObject*)(current_ + current_ptr()->GetTotalSize());
       }
      public:
       explicit OldPageIterator(const OldPage* page):
        RawObjectPointerIterator(),
        page_(page),
        current_(page->GetStartingAddress()){
       }
       ~OldPageIterator() override = default;

       const OldPage* page() const{
         return page_;
       }

       bool HasNext() const override{
         auto next = current_ptr();
#ifdef PSDN_DEBUG
         assert(page()->Contains(next->GetAddress()));
#endif//PSDN_DEBUG
         return next->GetPointerSize() > 0;
       }

       RawObject* Next() override{
         auto next = current_ptr();
         current_ += next->GetTotalSize();
         return next;
       }
     };
    protected:
     int64_t index_;
     uword start_;
     int64_t size_;
     OldPage* next_;
     OldPage* previous_;

     OldPage() = default;
     OldPage(int64_t index, uword start, int64_t size):
      index_(index),
      start_(start),
      size_(size),
      next_(nullptr),
      previous_(nullptr){
     }
    public:
     OldPage(const OldPage& rhs) = default;
     ~OldPage() = default;

     int64_t index() const{
       return index_;
     }

     int64_t size() const{
       return size_;
     }

     uword GetStartingAddress() const{
       return start_;
     }

     void* GetStartingAddressPointer() const{
       return (void*)GetStartingAddress();
     }

     uword GetEndingAddress() const{
       return GetStartingAddress() + size();
     }

     void* GetEndingAddressPointer() const{
       return (void*)GetEndingAddress();
     }

     bool HasNext() const{
       return next_ != nullptr;
     }

     OldPage* GetNext() const{
       return next_;
     }

     void SetNext(OldPage* page){
       next_ = page;
     }

     bool HasPrevious() const{
       return previous_ != nullptr;
     }

     OldPage* GetPrevious() const{
       return previous_;
     }

     void SetPrevious(OldPage* page){
       previous_ = page;
     }

     bool Contains(uword address) const{
       return GetStartingAddress() <= address
           && GetEndingAddress() >= address;
     }

     void VisitPointers(const std::function<bool(RawObject*)>& vis) const{
       OldPageIterator iter(this);
       while(iter.HasNext()){
         if(!vis(iter.Next()))
           return;
       }
     }

     OldPage& operator=(const OldPage& rhs) = default;

     friend std::ostream& operator<<(std::ostream& stream, const OldPage& val){
       return stream << "OldPage(index=" << val.index() << ", start=" << val.GetStartingAddressPointer() << ", size=" << Bytes(val.size()) << ")";
     }
   };
  private:
   OldPage* pages_;
  public:
   OldZone() = default;
   ~OldZone() override = default;

   OldZone& operator=(const OldZone& rhs){
     if(this == &rhs)
       return *this;
     Zone::operator=(rhs);
     return *this;
   }

   friend std::ostream& operator<<(std::ostream& stream, const OldZone& val){
     return stream << (Zone&)val;
   }
 };
}

#endif //POSEIDON_POSEIDON_ZONE_H
