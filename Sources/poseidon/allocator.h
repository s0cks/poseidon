#ifndef POSEIDON_ALLOCATOR_H
#define POSEIDON_ALLOCATOR_H

#include <glog/logging.h>

#include "poseidon/heap.h"

namespace poseidon{
 template<typename T>
 class Local;

 class LocalGroup;
 class Allocator{
  private:
   static LocalGroup* locals_;
   static uint64_t num_allocated_;
   static uint64_t num_locals_;

   static RawObject** NewLocalSlot();
  public:
   Allocator() = delete;
   Allocator(const Allocator& rhs) = delete;
   ~Allocator() = delete;

   static void Initialize();
   static void InitializeForThread();

   static inline uint64_t
   GetNumberOfObjectsAllocated(){
     return num_allocated_;
   }

   static inline uint64_t
   GetNumberOfLocals(){
     return num_locals_;
   }

   static inline uword
   Allocate(int64_t size){
     auto heap = Heap::GetCurrentThreadHeap();
#ifdef PSDN_DEBUG
     assert(heap != nullptr);
#endif//PSDN_DEBUG
     return heap->Allocate(size);
   }

   template<typename T>
   static Local<T> AllocateLocal(){
     num_locals_++;
     return Local<T>(NewLocalSlot());
   }

   static inline void*
   New(size_t size){
     return ((RawObject*)Allocate(size))->GetPointer();//TODO: fix
   }

   template<typename T>
   static inline T*
   New(){
     return (T*)New(sizeof(T));
   }

   static void VisitLocals(RawObjectVisitor* vis);
   static void VisitLocals(const std::function<bool(RawObject**)>& vis);
   static void VisitLocals(const std::function<bool(RawObject*)>& vis);
   static void VisitLocals(RawObjectPointerVisitor* vis);

   Allocator& operator=(const Allocator& rhs) = delete;
 };
}

#endif //POSEIDON_ALLOCATOR_H