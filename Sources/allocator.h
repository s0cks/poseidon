#ifndef POSEIDON_ALLOCATOR_H
#define POSEIDON_ALLOCATOR_H

#include <glog/logging.h>
#include "heap.h"
#include "poseidon.h"

namespace poseidon{
  template<typename T>
  class Local;

  class LocalGroup;
  class Allocator{
    friend class Finalizer;//TODO: remove
   private:
    static MemoryRegion* region_;
    static Heap* new_;
    static Heap* old_;

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

    static Heap* GetNewSpace(){
      return new_;
    }

    static Heap* GetOldSpace(){
      return old_;
    }

    static RawObject* AllocateRawObject(const uint64_t& size){
      auto raw = GetNewSpace()->AllocateRawObject(size);
      raw->SetNewBit();
      num_allocated_++;
      return raw;
    }

    template<typename T>
    static Local<T> AllocateLocal(){
      num_locals_++;
      return Local<T>(NewLocalSlot());
    }

    static void VisitLocals(RawObjectPointerPointerVisitor* vis);
    static void VisitLocals(ObjectPointerVisitor* vis);

    static void Clear(){
      GetNewSpace()->Clear();
      GetOldSpace()->Clear();
    }

    Allocator& operator=(const Allocator& rhs) = delete;
  };
}

#endif //POSEIDON_ALLOCATOR_H