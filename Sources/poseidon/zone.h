#ifndef POSEIDON_POSEIDON_ZONE_H
#define POSEIDON_POSEIDON_ZONE_H

#include "poseidon/flags.h"
#include "poseidon/bitset.h"
#include "poseidon/freelist.h"
#include "poseidon/semispace.h"
#include "poseidon/memory_region.h"

namespace poseidon{
 class NewPage;
 class Zone : public Section{
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
  protected:
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
     stream << "total_size=" << Bytes(zone.size());
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
   uword fromspace_;
   uword tospace_;
   int64_t semisize_;
  public:
   NewZone() = default;
   NewZone(uword start, int64_t size):
    Zone(start, size),
    fromspace_(start),
    tospace_(start + CalculateSemispaceSize(size)),
    semisize_(CalculateSemispaceSize(size)){
   }
   NewZone(MemoryRegion* region, int64_t offset, int64_t size):
     NewZone(region->GetStartingAddress() + offset, size){
   }
   ~NewZone() override = default;

   int64_t GetNumberOfBytesAllocated() const{
     return static_cast<int64_t>(GetCurrentAddress() - Zone::GetStartingAddress());
   }

   uword tospace() const{
     return tospace_;
   }

   uword fromspace() const{
     return fromspace_;
   }

   int64_t semisize() const{
     return semisize_;
   }

  /**
   * Swaps the from_ & to_ {@link Semispace}s in this {@link Zone}.
   *
   * Called during collection time.
   */
   virtual void SwapSpaces(){
     std::swap(fromspace_, tospace_);
     current_ = fromspace_;
   }

   uword Allocate(int64_t size) override{
     auto total_size = size + sizeof(RawObject);
     if((current_ + total_size) > (fromspace_ + tospace_)){
       return 0;//TODO: collect memory
     }

     if((current_ + total_size) > (fromspace_ + tospace_)){
       LOG(FATAL) << "insufficient memory.";
       return 0;
     }

     auto next = (void*)current_;
     current_ += total_size;
     auto ptr = new (next)RawObject();
     ptr->SetPointerSize(size);
     return ptr->GetAddress();
   }

   NewZone& operator=(const NewZone& rhs){
     if(this == &rhs)
       return *this;
     Zone::operator=(rhs);
     fromspace_ = rhs.fromspace_;
     tospace_ = rhs.tospace_;
     return *this;
   }

   friend std::ostream& operator<<(std::ostream& stream, const NewZone& val){
     return stream << (Zone&)val;
   }
 };

 class OldPage : public Section{
   friend class OldZone;
  public:
   static constexpr const int64_t kDefaultPageSize = 256 * 1024;

   class OldPageIterator : public RawObjectPointerIterator{
    private:
     const OldPage* page_;
     uword current_;

     inline RawObject* current_ptr() const{
       return (RawObject*)current_;
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
   OldPage* next_;
   OldPage* previous_;

   int64_t index_;
   uword start_;
   uword current_;
   int64_t size_;

   FreeList free_list_;

   OldPage():
    next_(nullptr),
    previous_(nullptr),
    index_(0),
    start_(0),
    current_(0),
    size_(0),
    free_list_(){
   }
   OldPage(int64_t index, uword start, int64_t size):
     next_(nullptr),
     previous_(nullptr),
     index_(index),
     start_(start),
     current_(start),
     size_(size),
     free_list_(start, size){
   }
  public:
   OldPage(const OldPage& rhs) = default;
   ~OldPage() override = default;

   int64_t index() const{
     return index_;
   }

   int64_t size() const override{
     return size_;
   }

   uword GetStartingAddress() const override{
     return start_;
   }

   uword GetCurrentAddress() const{
     return current_;
   }

   void* GetCurrentAddressPointer() const{
     return (void*)GetCurrentAddress();
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

   uword Allocate(int64_t size) override{
     auto total_size = size + static_cast<int64_t>(sizeof(RawObject));

     uword address;
     if((address = free_list_.TryAllocate(total_size)) != 0){
       auto raw_ptr = new ((void*)address)RawObject();
       raw_ptr->SetPointerSize(size);
       return raw_ptr->GetAddress();
     }

     DLOG(ERROR) << "failed to allocate " << Bytes(total_size) << " in " << (*this);
     return 0;
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

 static constexpr const int64_t kDefaultOldZoneSize = 128 * kMB;
 class OldZone : public Zone{
  protected:
   OldPage* pages_;
   BitSet table_;

   static inline int64_t
   GetNumberOfPages(const int64_t& size){
     PSDN_ASSERT(size % OldPage::kDefaultPageSize == 0);
     return size / OldPage::kDefaultPageSize;
   }

   static inline OldPage*
   GetPages(const uword& start, const int64_t& size){
     auto num_pages = GetNumberOfPages(size);

     OldPage* page = nullptr;
     do{
       auto offset = num_pages * OldPage::kDefaultPageSize;
       auto new_page = new OldPage(num_pages, start + offset, OldPage::kDefaultPageSize);
       new_page->SetNext(page);
       if(page)
         page->SetPrevious(new_page);
       page = new_page;
     } while(--num_pages >= 0);
     return page;
   }

   inline bool
   IsMarked(int64_t index) const{
     return table_.Test(index);
   }

   inline bool
   IsMarked(const OldPage* page) const{
     PSDN_ASSERT(page != nullptr);
     return IsMarked(page->index());
   }

   inline void
   MarkPage(const OldPage* page){
     PSDN_ASSERT(page != nullptr);
     GCLOG(1) << "marking " << (*page);
     return table_.Set(page->index(), true);
   }

   inline void
   UnmarkPage(const OldPage* page){
     PSDN_ASSERT(page != nullptr);
     GCLOG(1) << "unmarking " << (*page);
     return table_.Set(page->index(), false);
   }
  public:
   OldZone() = default;
   explicit OldZone(uword start):
     Zone(start, kDefaultOldZoneSize),
     table_(kDefaultOldZoneSize / OldPage::kDefaultPageSize),
     pages_(GetPages(start, kDefaultOldZoneSize)){
   }
   explicit OldZone(MemoryRegion* region, int64_t offset = 0):
     OldZone(region->GetStartingAddress() + offset){
   }
   ~OldZone() override = default;

   BitSet table() const{
     return table_;
   }

   uword Allocate(int64_t size) override{
     PSDN_ASSERT(pages_ != nullptr);
     auto page = pages_;
     do{
       uword result = 0;
       if((result = page->Allocate(size)) != 0){
         MarkPage(page);
         return result;
       }

       page = page->GetNext();
     } while(page != nullptr);

     //TODO: major collection
     LOG(ERROR) << "cannot allocate object of " << Bytes(size) << " size in " << (*this) << ".";
     return 0;
   }

   int64_t GetNumberOfPages() const{
     return GetNumberOfPages(kDefaultOldZoneSize);
   }

   void VisitAllPages(const std::function<bool(OldPage*)>& vis){
     auto page = pages_;
     while(page != nullptr){
       if(!vis(page))
         return;
       page = page->GetNext();
     }
   }

   void VisitPages(const std::function<bool(OldPage*)>& vis) const{
     auto page = pages_;
     while(page != nullptr){
       if(!vis(page))
         return;
       page = page->GetNext();
     }
   }

   void VisitMarkedPages(const std::function<bool(OldPage*)>& vis){
     auto page = pages_;
     while(page != nullptr){
       if(IsMarked(page) && !vis(page))
         return;
       page = page->GetNext();
     }
   }

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
