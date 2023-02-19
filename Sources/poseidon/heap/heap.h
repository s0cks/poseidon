#ifndef POSEIDON_HEAP_H
#define POSEIDON_HEAP_H

#include "poseidon/flags.h"
#include "poseidon/utils.h"

#include "poseidon/zone/zone.h"
#include "poseidon/zone/new_zone.h"
#include "poseidon/zone/old_zone.h"

#include "poseidon/platform/os_thread.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon{
 class Heap : public Region {
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
   NewZone* new_zone_;
   OldZone* old_zone_;

   uword AllocateNewObject(int64_t size);
   uword AllocateOldObject(int64_t size);
   uword AllocateLargeObject(int64_t size);
  public:
   explicit Heap(const MemoryRegion& region):
    Region(region.GetStartingAddress(), region.GetSize()),
    new_zone_(NewZone::New(MemoryRegion::Subregion(region, 0, flags::GetNewZoneSize()))),
    old_zone_(new OldZone(MemoryRegion::Subregion(region, flags::GetNewZoneSize(), flags::GetOldZoneSize()))) {
   }
   Heap(const Heap& rhs) = default;
   ~Heap() override = default;

   NewZone* new_zone() const {
     return new_zone_;
   }

   OldZone* old_zone() const {
     return old_zone_;
   }

   Pointer* TryAllocatePointer(word size);
   uword TryAllocateBytes(word size);
   uword TryAllocateClassBytes(Class* cls);

   template<typename T>
   T* TryAllocate() {
     return TryAllocateBytes(sizeof(T));
   }

   template<class T>
   T* TryAllocateClass() {
     return TryAllocateClassBytes(T::GetClass());
   }

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

     MemoryRegion region(flags::GetTotalInitialHeapSize());
     if(!region.Protect(MemoryRegion::kReadWrite))
       LOG(FATAL) << "cannot protect: " << region;

     auto heap = new Heap(region);
     SetCurrentThreadHeap(heap);//TODO: refactor.
     //TODO: print NewZone
     //TODO: print OldZone
   }

   static inline ObjectSize GetHeaderSize() {
     return sizeof(Heap);
   }

   static Heap* From(const MemoryRegion& region);
 };
}

#endif //POSEIDON_HEAP_H
