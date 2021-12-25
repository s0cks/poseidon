#ifndef POSEIDON_HEAP_H
#define POSEIDON_HEAP_H

#include "common.h"
#include "raw_object.h"
#include "memory_region.h"

namespace poseidon{
  class Heap;
  class ObjectPointerVisitor;
  class Semispace{
    friend class Heap;
   private:
    class SemispaceIterator : public RawObjectPointerIterator{
     private:
      Semispace* semispace_;
      uword ptr_;
     public:
      explicit SemispaceIterator(Semispace* semispace):
        semispace_(semispace),
        ptr_(semispace->GetStartAddress()){
      }
      ~SemispaceIterator() override = default;

      Semispace* GetSemispace() const{
        return semispace_;
      }

      bool HasNext() const override{
        auto raw = (RawObject*)ptr_;
        auto next = ptr_ + raw->GetTotalSize();
        return raw->GetPointerSize() > 0
            && GetSemispace()->Contains(next);
      }

      RawObject* Next() override{
        auto next = (RawObject*)ptr_;
        ptr_ += next->GetTotalSize();
        return next;
      }
    };

    Heap* heap_;
    uword start_;
    uword current_;
    uword size_;

    Semispace(Heap* heap, uword start, uword current, uword size):
      heap_(heap),
      start_(start),
      current_(current),
      size_(size){
    }
    Semispace(Heap* heap, uword start, uword size):
      Semispace(heap, start, start, size){
    }
   public:
    Semispace():
      heap_(nullptr),
      start_(0),
      current_(0),
      size_(0){
    }
    Semispace(const Semispace& rhs) = default;
    ~Semispace() = default;

    Heap* GetHeap() const{
      return heap_;
    }

    uword GetSize() const{
      return size_;
    }

    uword GetStartAddress() const{
      return start_;
    }

    uword GetCurrentAddress() const{
      return current_;
    }

    uword GetEndAddress() const{
      return GetStartAddress() + GetSize();
    }

    bool Contains(const uword& addr) const{
      return GetStartAddress() <= addr
          && GetEndAddress() >= addr;
    }

    void* GetPointer() const{
      return (void*)start_;
    }

    uint64_t GetTotalBytes() const{
      return GetEndAddress() - GetStartAddress();
    }

    uint64_t GetAllocatedBytes() const{
      return GetCurrentAddress() - GetStartAddress();
    }

    double GetAllocatedPercentage() const{
      return GetPercentageOf(GetAllocatedBytes(), GetTotalBytes());
    }

    uint64_t GetUnallocatedBytes() const{
      return GetTotalBytes() - GetAllocatedBytes();
    }

    double GetUnallocatedPercentage() const{
      return GetPercentageOf(GetUnallocatedBytes(), GetTotalBytes());
    }

    RawObject* AllocateRawObject(const uint64_t& size);
    void VisitRawObjectPointers(RawObjectPointerVisitor* vis);
    void VisitObjectPointers(ObjectPointerVisitor* vis);
    void VisitMarkedRawObjectPointers(RawObjectPointerVisitor* vis);

    bool IsEmpty() const{
      return GetCurrentAddress() == GetStartAddress();
    }

    void Clear(){
      memset((void*)start_, 0, size_);
      current_ = start_;
    }

    Semispace& operator=(const Semispace& rhs) = default;
  };

  class Heap{
   public:
    class HeapStats{
      friend class Heap;
     private:
      Space space_;
      uint64_t total_bytes_;
      uint64_t allocated_bytes_;

      HeapStats(const Space& space, const uint64_t& total, const uint64_t& allocated):
        space_(space),
        total_bytes_(total),
        allocated_bytes_(allocated){
      }
     public:
      HeapStats() = default;
      HeapStats(const HeapStats& rhs) = default;
      ~HeapStats() = default;

      Space GetSpace() const{
        return space_;
      }

      uint64_t GetTotalBytes() const{
        return total_bytes_;
      }

      uint64_t GetAllocatedBytes() const{
        return allocated_bytes_;
      }

      double GetAllocatedPercentage() const{
        return GetPercentageOf(GetAllocatedBytes(), GetTotalBytes());
      }

      uint64_t GetUnallocatedBytes() const{
        return GetTotalBytes() - GetAllocatedBytes();
      }

      double GetUnallocatedPercentage() const{
        return GetPercentageOf(GetUnallocatedBytes(), GetTotalBytes());
      }

      HeapStats& operator=(const HeapStats& rhs){
        if(this == &rhs)
          return *this;
        space_ = rhs.space_;
        total_bytes_= rhs.total_bytes_;
        allocated_bytes_ = rhs.allocated_bytes_;
        return *this;
      }

      friend std::ostream& operator<<(std::ostream& stream, const HeapStats& stats){
        return stream << HumanReadableSize(stats.GetAllocatedBytes()) << "/" << HumanReadableSize(stats.GetTotalBytes()) << " (" << PrettyPrintPercentage(stats.GetAllocatedPercentage()) << ")";
      }
    };

    class HeapIterator : public RawObjectPointerIterator{
     private:
      Heap* heap_;
      uword ptr_;
     public:
      explicit HeapIterator(Heap* heap):
        RawObjectPointerIterator(),
        heap_(heap),
        ptr_(heap->GetStartAddress()){
      }
      ~HeapIterator() override = default;

      Heap* GetHeap() const{
        return heap_;
      }

      bool HasNext() const override{
        auto ptr = (RawObject*)ptr_;
        auto next = ptr_ + ptr->GetTotalSize();
        return ptr->GetPointerSize() > 0
            && GetHeap()->Contains(next);
      }

      RawObject* Next() override{
        auto next = (RawObject*)ptr_;
        ptr_ += next->GetTotalSize();
        return next;
      }
    };
   private:
    Space space_;
    MemoryRegion region_;
    Semispace from_;
    Semispace to_;
   public:
    Heap() = default;
    explicit Heap(const Space& space, const MemoryRegion& region);
    Heap(const Heap& rhs) = default;
    ~Heap() = default;

    Space GetSpace() const{
      return space_;
    }

    MemoryRegion& GetRegion(){
      return region_;
    }

    Semispace& GetFromSpace(){
      return from_;
    }

    Semispace& GetToSpace(){
      return to_;
    }

    RawObject* AllocateRawObject(const uint64_t& size){
      return from_.AllocateRawObject(size);
    }

    uword GetStartAddress() const{
      return region_.GetStartAddress();
    }

    uword GetEndAddress() const{
      return region_.GetEndAddress();
    }

    bool Contains(const uword& address) const{
      return region_.Contains(address);
    }

    uint64_t GetTotalBytes() const{
      return region_.GetSize();
    }

    uint64_t GetAllocatedBytes() const{
      return from_.GetAllocatedBytes() + to_.GetAllocatedBytes();
    }

    uint64_t GetUnallocatedBytes() const{
      return from_.GetUnallocatedBytes() + to_.GetUnallocatedBytes();
    }

    bool IsEmpty() const{
      return from_.IsEmpty() && to_.IsEmpty();
    }

    HeapStats GetStats() const{
      return {GetSpace(), GetTotalBytes(), GetAllocatedBytes()};
    }

    void SwapSpaces();
    void VisitRawObjectPointers(RawObjectPointerVisitor* vis);
    void VisitObjectPointers(ObjectPointerVisitor* vis);
    void VisitMarkedRawObjectPointers(RawObjectPointerVisitor* vis);
    Heap& operator=(const Heap& rhs) = default;
  };
}

#endif //POSEIDON_HEAP_H
