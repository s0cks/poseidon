#ifndef POSEIDON_HEAP_H
#define POSEIDON_HEAP_H

#include "poseidon/flags.h"
#include "poseidon/utils.h"

#include "poseidon/heap/zone.h"
#include "poseidon/heap/new_zone.h"
#include "poseidon/heap/old_zone.h"

#include "poseidon/platform/os_thread.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon{
 class Heap{
   friend class Scavenger;
   friend class Compactor;
   friend class Allocator;

   friend class HeapTest;
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
   }

   explicit Heap(MemoryRegion* region, int64_t new_zone_size = GetNewZoneSize(), int64_t old_zone_size = GetOldZoneSize(), int64_t old_page_size = GetOldPageSize()):
    region_(region),
    new_zone_(new NewZone(region, new_zone_size)),
    old_zone_(new OldZone(region, new_zone_size, old_zone_size, old_page_size)){
   }

   Heap():
    Heap(new MemoryRegion(GetTotalInitialHeapSize())){
   }

   uword AllocateNewObject(int64_t size);
   uword AllocateOldObject(int64_t size);
   uword AllocateLargeObject(int64_t size);
  public:
   Heap(const Heap& rhs) = default;
   virtual ~Heap(){
     delete new_zone_;
     delete old_zone_;
   }

   MemoryRegion* region() const{
     return region_;
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
     ResetCurrentThreadHeap();
   }

   static inline bool
   CurrentThreadHasHeap(){
     return pthread_getspecific(kThreadKey) != nullptr;
   }

   static inline void
   ResetCurrentThreadHeap(){
     if(CurrentThreadHasHeap()){
       auto heap = GetCurrentThreadHeap();
       delete heap;
       SetCurrentThreadHeap(nullptr);
     }

     auto heap = new Heap();
     SetCurrentThreadHeap(heap);//TODO: refactor.
     GCLOG(10) << "new-zone: " << (heap->new_zone());
     GCLOG(10) << " - from: " << ((void*)heap->new_zone()->fromspace()) << " (" << Bytes(heap->new_zone()->semisize()) << ").";
     GCLOG(10) << " - to: " << ((void*)heap->new_zone()->tospace()) << " (" << Bytes(heap->new_zone()->semisize()) << ").";
     GCLOG(10) << "old-zone: " << (heap->old_zone());
   }
 };
}

#endif //POSEIDON_HEAP_H
