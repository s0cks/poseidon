#ifndef POSEIDON_ALLOCATOR_H
#define POSEIDON_ALLOCATOR_H

#include <glog/logging.h>

#include "poseidon/heap/heap.h"

namespace poseidon{
 class Allocator{
   friend class Scavenger;
  public:
   Allocator() = delete;
   Allocator(const Allocator& rhs) = delete;
   ~Allocator() = delete;

   static void Initialize();

   static inline uword
   Allocate(int64_t size){
     auto heap = Heap::GetForCurrentThread();
     PSDN_ASSERT(heap != nullptr);
     return heap->TryAllocateBytes(size);
   }

   static inline void*
   New(size_t size){
     auto ptr = (Pointer*)Allocate(static_cast<int64_t>(size));
     return ptr != nullptr ? ptr->GetPointer() : nullptr;
   }

   template<typename T>
   static inline T*
   New(){
     auto ptr = (Pointer*)Allocate(static_cast<int64_t>(sizeof(T)));
     return ptr != nullptr ? (T*)ptr->GetPointer() : nullptr;
   }

   template<typename T>
   static inline T*
   New(const T& value){
     auto ptr = (Pointer*)Allocate(static_cast<int64_t>(sizeof(T)));
     if(ptr != nullptr)
       *((T*)ptr->GetPointer()) = value;
     return ptr != nullptr ? (T*)ptr->GetPointer() : nullptr;
   }

   Allocator& operator=(const Allocator& rhs) = delete;
 };
}

#endif //POSEIDON_ALLOCATOR_H