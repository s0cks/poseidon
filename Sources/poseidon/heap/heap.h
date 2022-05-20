#ifndef POSEIDON_HEAP_H
#define POSEIDON_HEAP_H

#include "poseidon/flags.h"
#include "poseidon/utils.h"

#include "poseidon/heap/zone.h"
#include "poseidon/heap/zone_new.h"
#include "poseidon/heap/zone_old.h"

#include "poseidon/platform/os_thread.h"
#include "poseidon/platform/memory_region.h"

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

   uword AllocateNewObject(int64_t size);
   uword AllocateOldObject(int64_t size);
   uword AllocateLargeObject(int64_t size);
  public:
   Heap():
    region_(new MemoryRegion(GetTotalInitialHeapSize())),
    new_zone_(new NewZone(region_, GetNewZoneSize())),
    old_zone_(new OldZone(region_, GetNewZoneSize(), GetOldZoneSize(), GetOldPageSize())){
     if(!region_->Protect(MemoryRegion::kReadWrite)){//TODO: remove
       LOG(FATAL) << "failed to protect Heap " << region_;
     }
   }
   Heap(const Heap& rhs) = default;
   virtual ~Heap(){
     delete new_zone_;
     delete old_zone_;
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
     GCLOG(10) << "new-zone: " << (heap->new_zone());
     GCLOG(10) << " - from: " << ((void*)heap->new_zone()->fromspace()) << " (" << Bytes(heap->new_zone()->semisize()) << ").";
     GCLOG(10) << " - to: " << ((void*)heap->new_zone()->tospace()) << " (" << Bytes(heap->new_zone()->semisize()) << ").";
     GCLOG(10) << "old-zone: " << (heap->old_zone());
   }
 };
}

#endif //POSEIDON_HEAP_H
