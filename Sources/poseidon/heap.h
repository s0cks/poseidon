#ifndef POSEIDON_HEAP_H
#define POSEIDON_HEAP_H

#include "poseidon/zone.h"
#include "poseidon/flags.h"
#include "poseidon/memory_region.h"

namespace poseidon{
 static inline uint64_t
 GetNewZoneSize(){
   return FLAGS_new_zone_size;
 }

 static inline uint64_t
 GetOldZoneSize() {
   return FLAGS_old_zone_size;
 }

 static inline uint64_t
 GetTotalHeapSize(){
   return GetNewZoneSize() + GetOldZoneSize();
 }

 static inline uint64_t
 GetHeapPageSize(){
   return FLAGS_heap_page_size;
 }

 static inline uint64_t
 GetLargeObjectSize(){
   return FLAGS_large_object_size;
 }

 class HeapPage{
   friend class Compactor;
  private:
   class HeapPageIterator : public RawObjectPointerIterator{
    private:
     const HeapPage* page_;
     uword current_;

     RawObject* current_ptr() const{
       return (RawObject*)current_;
     }

     RawObject* next_ptr() const{
       return (RawObject*)(current_ + current_ptr()->GetTotalSize());
     }
    public:
     explicit HeapPageIterator(const HeapPage* page):
         page_(page),
         current_(page->GetStartingAddress()){
     }
     ~HeapPageIterator() override = default;

     const HeapPage* page() const{
       return page_;
     }

     bool HasNext() const override{
       auto next = next_ptr();
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
  private:
   HeapPage* next_;
   HeapPage* previous_;

   MemoryRegion region_;
   uword current_;

   inline void SetCurrent(uword address){
#ifdef PSDN_DEBUG
     assert(Contains(address));
#endif//PSDN_DEBUG
     current_ = address;
   }
  public:
   explicit HeapPage(uint64_t size = GetHeapPageSize()):
    next_(nullptr),
    previous_(nullptr),
    region_(size),
    current_(0){
     if(!region_.Protect(MemoryRegion::kReadWrite))
       LOG(FATAL) << "cannot protect HeapPage MemoryRegion.";
     current_ = region_.GetStartAddress();
   }
   ~HeapPage() = default;

   HeapPage* next() const{
     return next_;
   }

   bool HasNext() const{
     return next_ != nullptr;
   }

   void SetNext(HeapPage* page){
     next_ = page;
   }

   HeapPage* previous() const{
     return previous_;
   }

   bool HasPrevious() const{
     return previous_ != nullptr;
   }

   void SetPrevious(HeapPage* page){
     previous_ = page;
   }

   uint64_t size() const{
     return region_.GetSize();
   }

   uword GetStartingAddress() const{
     return region_.GetStartAddress();
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
     return GetStartingAddress() + size();
   }

   void* GetEndingAddressPointer() const{
     return (void*)GetEndingAddress();
   }

   bool Contains(uword address) const{
     return GetStartingAddress() <= address
         && GetEndingAddress() >= address;
   }

   uint64_t GetNumberOfBytesAllocated() const{
     return GetCurrentAddress() - GetStartingAddress();
   }

   bool IsFull() const{
     return GetCurrentAddress() == GetEndingAddress();
   }

   RawObject* Allocate(uint64_t size);

   void VisitPointers(const std::function<bool(RawObject*)>& vis) const{
     HeapPageIterator iter(this);
     while(iter.HasNext()){
       if(!vis(iter.Next()))
         return;
     }
   }

   void VisitPointers(RawObjectVisitor* vis) const{
     HeapPageIterator iter(this);
     while(iter.HasNext()){
       if(!vis->Visit(iter.Next()))
         return;
     }
   }

   void VisitMarkedPointers(const std::function<bool(RawObject*)>& vis) const{
     HeapPageIterator iter(this);
     while(iter.HasNext()){
       auto next = iter.Next();
       if(next->IsMarked() && !vis(next))
         return;
     }
   }

   void VisitMarkedPointers(RawObjectVisitor* vis) const{
     HeapPageIterator iter(this);
     while(iter.HasNext()){
       auto next = iter.Next();
       if(next->IsMarked() && !vis->Visit(next))
         return;
     }
   }

   friend std::ostream& operator<<(std::ostream& stream, const HeapPage& val){
     stream << "HeapPage(";
     stream << "starting_address=" << val.GetStartingAddressPointer() << ", ";
     stream << "total_size=" << HumanReadableSize(val.size()) << ", ";
     stream << "allocated=" << HumanReadableSize(val.GetNumberOfBytesAllocated());
     stream << ")";
     return stream;
   }
 };

 class Heap{
  private:
   MemoryRegion* region_;
   Zone* new_zone_;
   Zone* old_zone_;

   HeapPage* pages_;
   uint32_t num_pages_;

   inline HeapPage*
   CreateNewHeapPage(HeapPage* parent){
     if((num_pages_ + 1) >= FLAGS_max_heap_pages){
       LOG(FATAL) << "cannot create new heap page, there is already " << (num_pages_ + 1) << "/" << FLAGS_max_heap_pages << " pages allocated.";
       return nullptr;
     }

     auto page = new HeapPage();
     if(parent != nullptr){
       page->SetNext(parent);
       parent->SetPrevious(page);
     }

     pages_ = page;
     num_pages_ += 1;
     return page;
   }

   inline HeapPage*
   GetCurrentPage() const{
     return pages_;
   }

   RawObject* AllocateNewObject(uint64_t size);
   RawObject* AllocateOldObject(uint64_t size);
  public:
   Heap():
    region_(new MemoryRegion(GetTotalHeapSize())),
    new_zone_(new Zone(region_, 0, GetNewZoneSize())),
    old_zone_(new Zone(region_, GetNewZoneSize(), GetOldZoneSize())),
    pages_(nullptr),
    num_pages_(0){
     if(!region_->Protect(MemoryRegion::kReadWrite))
       LOG(ERROR) << "cannot protect Heap MemoryRegion.";
   }
   Heap(const Heap& rhs) = default;
   ~Heap() = default;

   const MemoryRegion* region() const{
     return region_;
   }

   uword GetStartingAddress() const{
     return region_->GetStartAddress();
   }

   void* GetStartingAddressPointer() const{
     return (void*)GetStartingAddress();
   }

   uword GetEndingAddress() const{
     return region_->GetEndAddress();
   }

   void* GetEndingAddressPointer() const{
     return (void*)GetEndingAddress();
   }

   Zone* new_zone() const{
     return new_zone_;
   }

   Zone* old_zone() const{
     return old_zone_;
   }

   bool Contains(uword address) const{
     return region_->Contains(address);
   }

   void VisitPages(const std::function<bool(const HeapPage* page)>& vis) const{
     auto page = GetCurrentPage();
     while(page != nullptr){
       if(!vis(page))
         return;
     }
   }

   RawObject* AllocateObject(uint64_t size);

   Heap& operator=(const Heap& rhs) = delete;

   friend std::ostream& operator<<(std::ostream& stream, const Heap& heap){//TODO: implement
     return stream;
   }
 };

 class HeapPrinter{
  public:
   enum Flags : uint64_t{
     kNone = 0,
     kDetailed = 1 << 1,
     kHexDump = 1 << 2,

#ifdef PSDN_DEBUG
     kDefault = kDetailed | kHexDump,
#else
     kDefault = kNone,
#endif//PSDN_DEBUG
   };

   HeapPrinter() = delete;
   ~HeapPrinter() = delete;

   static void Print(Heap* heap, uint64_t flags = kDefault);
 };
}

#endif //POSEIDON_HEAP_H
