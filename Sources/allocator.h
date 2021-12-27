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
    static MemoryRegion region_;
    static Heap* eden_;
    static Heap* tenured_;
    static Heap* large_object_;
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

    static inline uword
    GetHeapSize(){
      return FLAGS_heap_size;
    }

    static inline Heap*
    GetEdenHeap(){
      return eden_;
    }

    static inline uword
    GetEdenSpaceOffset(){
      return 0;
    }

    static inline uword
    GetEdenSpaceSize(){
      return GetHeapSize() / 4;
    }

    static inline Heap*
    GetTenuredHeap(){
      return tenured_;
    }

    static inline uword
    GetTenuredSpaceOffset(){
      return GetEdenSpaceOffset() + GetEdenSpaceSize();
    }

    static inline uword
    GetTenuredSpaceSize(){
      return GetHeapSize() / 4;
    }

    static inline Heap*
    GetLargeObjectHeap(){
      return large_object_;
    }

    static inline uword
    GetLargeObjectSpaceOffset(){
      return GetTenuredSpaceOffset() + GetTenuredSpaceSize();
    }

    static inline uword
    GetLargeObjectSpaceSize(){
      return GetHeapSize() / 2;
    }

    static RawObject* AllocateRawObject(const uint64_t& size){
      auto raw = GetEdenHeap()->AllocateRawObject(size);
      raw->SetEdenBit();
      raw->SetTest(RawObject::kMagic);
      num_allocated_++;
      return raw;
    }

    template<typename T>
    static Local<T> AllocateLocal(){
      num_locals_++;
      return Local<T>(NewLocalSlot());
    }

    static void VisitLocals(RawObjectPointerPointerVisitor* vis);
    static void VisitLocals(RawObjectPointerVisitor* vis);
    static void VisitLocals(ObjectPointerVisitor* vis);

    Allocator& operator=(const Allocator& rhs) = delete;
  };
}

#endif //POSEIDON_ALLOCATOR_H