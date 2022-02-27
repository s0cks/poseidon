#ifndef POSEIDON_ALLOCATOR_H
#define POSEIDON_ALLOCATOR_H

#include <glog/logging.h>
#include "heap.h"

namespace poseidon{
  template<typename T>
  class Local;

  class LocalGroup;
  class Allocator{
    friend class Finalizer;//TODO: remove
   private:
    static Heap* heap_;

    static LocalGroup* locals_;
    static uint64_t num_allocated_;
    static uint64_t num_locals_;

    static RawObject** NewLocalSlot();
    static void FinalizeObject(RawObject* raw);
   public:
    Allocator() = delete;
    Allocator(const Allocator& rhs) =delete;
    ~Allocator() = delete;

    static void Initialize();

    static inline uint64_t
    GetNumberOfObjectsAllocated(){
      return num_allocated_;
    }

    static inline uint64_t
    GetNumberOfLocals(){
      return num_locals_;
    }

    static Heap* GetHeap(){
      return heap_;
    }

    static inline RawObject*
    AllocateNewObject(uint64_t size){
      auto raw = GetHeap()->AllocateNewObject(size);
#ifdef PSDN_DEBUG
      assert(raw != nullptr);
#endif//PSDN_DEBUG
      num_allocated_ += 1;
      return raw;
    }

    static inline RawObject*
    AllocateOldObject(uint64_t size){
      auto raw = GetHeap()->AllocateOldObject(size);
#ifdef PSDN_DEBUG
      assert(raw != nullptr);
#endif//PSDN_DEBUG
      num_allocated_ += 1;
      return raw;
    }

    template<typename T>
    static Local<T> AllocateLocal(){
      num_locals_++;
      return Local<T>(NewLocalSlot());
    }

    static void VisitLocals(RawObjectPointerPointerVisitor* vis);
    static void VisitLocals(const std::function<bool(RawObject*)>& vis);

    static void Clear(){

    }

    Allocator& operator=(const Allocator& rhs) = delete;
  };
}

#endif //POSEIDON_ALLOCATOR_H