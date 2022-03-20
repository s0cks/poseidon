#ifndef POSEIDON_HEAP_H
#define POSEIDON_HEAP_H

#include "poseidon/zone.h"
#include "poseidon/flags.h"
#include "poseidon/utils.h"
#include "poseidon/os_thread.h"
#include "poseidon/memory_region.h"

namespace poseidon{
 static constexpr const uint64_t kNewZonePageSize = 256 * 1024;

 static inline int64_t
 GetNewZoneSize(){
   return FLAGS_new_zone_size;
 }

 static inline int64_t
 GetTotalHeapSize(){
   return GetNewZoneSize() + kDefaultOldZoneSize;
 }

 static inline int64_t
 GetHeapPageSize(){
   return FLAGS_heap_page_size;
 }

 static inline int64_t
 GetLargeObjectSize(){
   return FLAGS_large_object_size;
 }

 class NewPage{
  public:
   class NewPageIterator : public RawObjectPointerIterator{
    private:
     const NewPage* page_;
     uword current_;

     inline RawObject* current_ptr() const{
       return (RawObject*)current_;
     }

     inline RawObject* next_ptr() const{
       return (RawObject*)(current_ + current_ptr()->GetTotalSize());
     }
    public:
     explicit NewPageIterator(const NewPage* page):
      RawObjectPointerIterator(),
      page_(page),
      current_(page->GetStartingAddress()){
     }
     ~NewPageIterator() override = default;

     const NewPage* page() const{
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

   static inline int64_t
   GetIndexFromPageDimensions(uword zone_start, uword start, int64_t page_size = kNewZonePageSize){
     auto index = (start - zone_start) / page_size;

     return index;
   }
  private:
   int64_t index_;
   uword start_;
   int64_t size_;
  public:
   NewPage(int64_t index, uword start, int64_t size):
    index_(index),
    start_(start),
    size_(size){
   }
   NewPage(const NewPage& rhs) = default;
   ~NewPage() = default;

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

   uword GetEndingAddress() const{
     return GetStartingAddress() + GetSize();
   }

   void* GetEndingAddressPointer() const{
     return (void*)GetEndingAddress();
   }

   bool Contains(uword address) const{
     return GetStartingAddress() <= address
         && GetEndingAddress() >= address;
   }

   void VisitPointers(const std::function<bool(RawObject*)>& vis) const{
     NewPageIterator iter(this);
     while(iter.HasNext()){
       if(!vis(iter.Next()))
         return;
     }
   }

   NewPage& operator=(const NewPage& rhs) = default;

   friend std::ostream& operator<<(std::ostream& stream, const NewPage& val){
     return stream << "NewPage(index=" << val.index() << ", start=" << val.GetStartingAddressPointer() << ", size=" << Bytes(val.GetSize()) << ")";
   }
 };

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
     current_ = region_.GetStartingAddress();
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
     return region_.size();
   }

   uword GetStartingAddress() const{
     return region_.GetStartingAddress();
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
     stream << "total_size=" << Bytes(val.size()) << ", ";
     stream << "allocated=" << Bytes(val.GetNumberOfBytesAllocated());
     stream << ")";
     return stream;
   }
 };

 class Heap : public AllocationSection{
   friend class HeapTest;
   friend class Scavenger;
   friend class Compactor;
   friend class Allocator;
  private:
   static pthread_key_t kThreadKey;

   static inline void
   SetCurrentThreadHeap(Heap* heap){
     int err;
     if((err = pthread_setspecific(kThreadKey, (const void*)heap)) != 0){
       LOG(ERROR) << "cannot set Heap ThreadLocal: " << strerror(err);
       return;
     }
     DLOG(INFO) << "set thread " << GetCurrentThreadName() << " Heap ThreadLocal to " << (*heap);
   }
  private:
   MemoryRegion* region_;
   NewZone* new_zone_;
   OldZone* old_zone_;

   HeapPage* pages_;
   int32_t num_pages_;

   Heap(MemoryRegion* region, NewZone* new_zone, OldZone* old_zone):
     region_(region),
     new_zone_(new_zone),
     old_zone_(old_zone),
     pages_(nullptr),
     num_pages_(0){
     if(!region_->Protect(MemoryRegion::kReadWrite))
       LOG(ERROR) << "cannot protect Heap MemoryRegion.";
   }

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

   uword AllocateNewObject(int64_t size);
   uword AllocateOldObject(int64_t size);
   uword AllocateLargeObject(int64_t size);
  public:
   explicit Heap(MemoryRegion* region = new MemoryRegion(GetTotalHeapSize()))://TODO: refactor
    Heap(region, new NewZone(region, 0, GetNewZoneSize()), new OldZone(region, GetNewZoneSize())){
   }
   Heap(const Heap& rhs) = default;
   ~Heap() override = default;

   const MemoryRegion* region() const{
     return region_;
   }

   int64_t size() const override{
     return static_cast<int64_t>(region_->size());
   }

   uword GetStartingAddress() const override{
     return region_->GetStartingAddress();
   }

   NewZone* new_zone() const{
     return new_zone_;
   }

   OldZone* old_zone() const{
     return old_zone_;
   }

   void VisitPages(const std::function<bool(const HeapPage* page)>& vis) const{
     auto page = GetCurrentPage();
     while(page != nullptr){
       if(!vis(page))
         return;
     }
   }

   uword Allocate(int64_t size) override;

   Heap& operator=(const Heap& rhs) = delete;

   friend std::ostream& operator<<(std::ostream& stream, const Heap& heap){//TODO: implement
     return stream;
   }

   static inline Heap*
   GetCurrentThreadHeap(){
     void* ptr = nullptr;
     if((ptr = pthread_getspecific(kThreadKey)) != nullptr)
       return (Heap*)ptr;
     LOG(WARNING) << "cannot get Heap ThreadLocal for thread " << GetCurrentThreadName() << ".";
     return nullptr;
   }

   /**
    *               ***Only Call From Main Thread***
    *
    * Initializes the Heap for the main thread.
    */
   static inline void
   Initialize(){
     int err;
     if((err = pthread_key_create(&kThreadKey, nullptr)) != 0){
       LOG(ERROR) << "failed to create Heap ThreadLocal: " << strerror(err);
       return;
     }
     DLOG(INFO) << "initialized Heap ThreadLocal.";
     SetCurrentThreadHeap(new Heap());//TODO: refactor.
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
