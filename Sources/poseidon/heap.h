#ifndef POSEIDON_HEAP_H
#define POSEIDON_HEAP_H

#include "poseidon/zone.h"
#include "poseidon/flags.h"
#include "poseidon/utils.h"
#include "poseidon/os_thread.h"
#include "poseidon/memory_region.h"

namespace poseidon{
 class Heap{
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
   }
  private:
   MemoryRegion* region_;
   NewZone* new_zone_;
   OldZone* old_zone_;

   Heap(MemoryRegion* region, NewZone* new_zone, OldZone* old_zone):
     region_(region),
     new_zone_(new_zone),
     old_zone_(old_zone){
     if(!region_->Protect(MemoryRegion::kReadWrite))
       LOG(ERROR) << "cannot protect Heap MemoryRegion.";
   }

   uword AllocateNewObject(int64_t size);
   uword AllocateOldObject(int64_t size);
   uword AllocateLargeObject(int64_t size);
  public:
   explicit Heap(MemoryRegion* region = new MemoryRegion(GetTotalInitialHeapSize()))://TODO: refactor
    Heap(region, new NewZone(*region, 0, GetNewZoneSize()), new OldZone(*region, GetNewZoneSize(), kDefaultOldZoneSize, OldPage::kDefaultPageSize)){
   }
   Heap(const Heap& rhs) = default;
   virtual ~Heap() = default;

   const MemoryRegion* region() const{
     return region_;
   }

   uword GetStartingAddress() const{
     return region_->GetStartingAddress();
   }

   uword GetEndingAddress() const{
     return region_->GetEndingAddress();
   }

   NewZone* new_zone() const{
     return new_zone_;
   }

   OldZone* old_zone() const{
     return old_zone_;
   }

   uword TryAllocate(int64_t size);

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
     auto heap = new Heap();
     SetCurrentThreadHeap(heap);//TODO: refactor.
     GCLOG(10) << "new-zone: " << (*heap->new_zone());
     GCLOG(10) << " - from: " << ((void*)heap->new_zone()->fromspace()) << " (" << Bytes(heap->new_zone()->semisize()) << ").";
     GCLOG(10) << " - to: " << ((void*)heap->new_zone()->tospace()) << " (" << Bytes(heap->new_zone()->semisize()) << ").";
     GCLOG(10) << "old-zone: " << (*heap->old_zone());
   }
 };
}

#endif //POSEIDON_HEAP_H
