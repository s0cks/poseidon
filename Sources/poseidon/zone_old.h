#ifndef POSEIDON_ZONE_OLD_H
#define POSEIDON_ZONE_OLD_H

#include "poseidon/zone.h"

namespace poseidon{
 class OldPage{
   friend class OldZone;
  public:
   class OldPageIterator : public RawObjectPointerIterator{
    private:
     const OldPage* page_;
     uword current_;

     inline uword current_address() const{
       return current_;
     }

     inline RawObject* current_ptr() const{
       return (RawObject*)current_address();
     }

     inline uword next_address() const{
       return current_ + current_ptr()->GetPointerSize();
     }

     inline RawObject* next_ptr() const{
       return (RawObject*)next_address();
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
       return current_address() < page()->GetEndingAddress();
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

   OldPage(int64_t index, uword start, int64_t size):
     next_(nullptr),
     previous_(nullptr),
     index_(index),
     start_(start),
     current_(start),
     size_(size){
   }
  public:
   OldPage() = delete;
   OldPage(const OldPage& rhs) = delete;
   virtual ~OldPage() = default;

   int64_t index() const{
     return index_;
   }

   int64_t GetSize() const{
     return size_;
   }

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

   uword GetEndingAddress() const{
     return GetStartingAddress() + GetSize();
   }

   void* GetEndingAddressPointer() const{
     return (void*)GetEndingAddress();
   }

   bool Contains(uword address) const{
     return address >= GetStartingAddress()
         && address <= GetEndingAddress();
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

   uword TryAllocate(int64_t size){
     auto total_size = size + static_cast<int64_t>(sizeof(RawObject));

     uword address = current_;
     current_ += total_size;
     if(address != 0){
       auto raw_ptr = new ((void*)address)RawObject();
       raw_ptr->SetPointerSize(size);
       return address;
     }

     GCLOG(1) << "failed to allocate " << Bytes(total_size) << " in " << (*this);
     return 0;
   }

   void VisitPointers(const std::function<bool(RawObject*)>& vis) const{
     OldPageIterator iter(this);
     while(iter.HasNext()){
       auto next = iter.Next();
       if(next->IsOld() && next->GetPointerSize() > 0 && !vis(next))
         return;
     }
   }

   int64_t GetTotalBytesAllocated() const{
     return static_cast<int64_t>(GetCurrentAddress() - GetStartingAddress());
   }

   OldPage& operator=(const OldPage& rhs) = delete;

   friend std::ostream& operator<<(std::ostream& stream, const OldPage& val){
     return stream << "OldPage(index=" << val.index() << ", start=" << val.GetStartingAddressPointer() << ", size=" << Bytes(val.GetSize()) << ", end=" << val.GetEndingAddressPointer() << ").";
   }
 };

 class OldZone : public Zone{
   friend class OldZoneTest;

   friend class SerialSweeper;
   friend class ParallelSweeper;
  protected:
   int64_t num_pages_;
   OldPage** pages_;
   BitSet marked_;
   FreeList free_list_;

   static inline int64_t
   GetNumberOfPages(int64_t size, int64_t page_size){
     PSDN_ASSERT(IsPow2(size));
     PSDN_ASSERT(IsPow2(page_size));
     PSDN_ASSERT(size % page_size == 0);
     return size / page_size;
   }

   static inline OldPage*
   GetPages(const uword& start, int64_t size, int64_t page_size){
     PSDN_ASSERT((size % page_size) == 0);
     PSDN_ASSERT((size % kWordSize) == 0);

     auto num_pages = GetNumberOfPages(size, page_size);
     int64_t index = 0;

     auto first = new OldPage(index, start, page_size);
     index++;
     OldPage* current = first;

     do{
       DLOG(INFO) << "creating page " << index << "/" << num_pages;
       auto offset = index * page_size;
       auto new_page = new OldPage(index, start + offset, page_size);

       // link
       current->SetNext(new_page);
       new_page->SetPrevious(current);

       current = new_page;
       index++;
     } while(index < num_pages);
     return first;
   }

   inline bool
   IsMarked(int64_t index) const{
     return marked_.Test(index);
   }

   inline bool
   IsMarked(const OldPage* page) const{
     PSDN_ASSERT(page != nullptr);
     return IsMarked(page->index());
   }

   inline void
   MarkPage(const OldPage* page){
     PSDN_ASSERT(page != nullptr);
     DLOG(INFO) << "marking " << (*page);
     return marked_.Set(page->index(), true);
   }

   inline void
   UnmarkPage(const OldPage* page){
     PSDN_ASSERT(page != nullptr);
     DLOG(INFO) << "unmarking " << (*page);
     return marked_.Set(page->index(), false);
   }

   static inline int64_t
   CalculateTableSize(int64_t size, int64_t page_size){
     return size / page_size;
   }

   inline OldPage* pages(int64_t idx){
     return pages_[idx];
   }
  public:
   explicit OldZone(uword start, int64_t size, int64_t page_size):
     Zone(start, size),
     num_pages_(0),
     pages_(nullptr),
     marked_(CalculateTableSize(size, page_size)),
     free_list_(start, size){
     PSDN_ASSERT(IsPow2(size));
     PSDN_ASSERT(IsPow2(page_size));

     auto npages = GetNumberOfPages(size, page_size);
     PSDN_ASSERT(npages > 0);
     PSDN_ASSERT(IsPow2(npages));
     DLOG(INFO) << "creating " << npages << " pages....";

     if((pages_ = new OldPage*[npages]) == nullptr){
       LOG(FATAL) << "failed to allocate OldPages list.";
       return;
     }

     for(auto idx = 0; idx < npages; idx++){
       auto page_offset = start + (idx * page_size);
       pages_[idx] = new OldPage(idx, page_offset, page_size);
     }
     num_pages_ = npages;
   }
   explicit OldZone(MemoryRegion* region, int64_t offset, int64_t size, int64_t page_size):
     OldZone(region->GetStartingAddress() + offset, size, page_size){
   }
   ~OldZone() override{
     if(pages_)
       free(pages_);
   }

   FreeList* free_list(){//TODO: visible for testing
     return &free_list_;
   }

   uword TryAllocate(int64_t size) override;

   int64_t GetNumberOfPages() const{
     return num_pages_;
   }

   void VisitAllPages(const std::function<bool(OldPage*)>& vis){
     for(auto idx = 0; idx < num_pages_; idx++){
       auto page = pages_[idx];
       if(!vis(page))
         return;
     }
   }

   void VisitMarkedPages(const std::function<bool(OldPage*)>& vis){
     for(auto idx = 0; idx < num_pages_; idx++){
       auto page = pages_[idx];
       if(IsMarked(page) && !vis(page))
         return;
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

#endif//POSEIDON_ZONE_OLD_H