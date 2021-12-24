#ifndef POSEIDON_ALLOCATOR_H
#define POSEIDON_ALLOCATOR_H

#include <glog/logging.h>
#include "heap.h"
#include "local.h"
#include "object.h"
#include "poseidon.h"

namespace poseidon{
  template<typename T>
  class Local;

  class LocalGroup;
  class Allocator{
   private:
    static MemoryRegion region_;
    static Heap* eden_;
    static Heap* tenured_;
    static Heap* large_object_;
    static LocalGroup* locals_;

    static RawObject** NewLocalSlot();
   public:
    Allocator() = delete;
    Allocator(const Allocator& rhs) =delete;
    ~Allocator() = delete;

    static void Initialize();

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

    static void* Allocate(const uint64_t& size){
      auto raw = GetEdenHeap()->AllocateRawObject(size);
      raw->SetEdenBit();
      return raw->GetObjectPointer();
    }

    static void* Allocate(Class* cls){
      return Allocate(cls->GetAllocationSize());
    }

    template<typename T>
    static T* Allocate(Class* cls){
      return (T*)Allocate(cls);
    }

    template<typename T>
    static Local<T> AllocateLocal(Class* cls){
      auto raw = GetEdenHeap()->AllocateRawObject(cls->GetAllocationSize());
      raw->SetEdenBit();
      auto slot = NewLocalSlot();
      (*slot) = raw;
      return Local<T>(slot);
    }

    template<typename T>
    static Local<T> AllocateLocal(const T& value){
      auto slot = NewLocalSlot();
      (*slot) = GetEdenHeap()->AllocateRawObject(sizeof(T));
      memcpy((*slot)->GetObjectPointer(), &value, sizeof(value));
      return Local<T>(slot);
    }

    static void VisitLocals(RawObjectPointerPointerVisitor* vis);
    static void VisitLocals(RawObjectPointerVisitor* vis);
    static void VisitLocals(ObjectPointerVisitor* vis);

    Allocator& operator=(const Allocator& rhs) = delete;
  };
}

#endif //POSEIDON_ALLOCATOR_H