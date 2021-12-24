#include <glog/logging.h>

#include "heap.h"
#include "utils.h"
#include "object.h"
#include "raw_object.h"

namespace poseidon{
  static inline uword
  GetSemispaceSize(const MemoryRegion& region){
    return region.GetSize() / 2;
  }

  static inline uword
  GetFromSemispaceStart(const MemoryRegion& region){
    return region.GetStartAddress();
  }

  static inline uword
  GetToSemispaceStart(const MemoryRegion& region){
    return region.GetStartAddress() + GetSemispaceSize(region);
  }

  Heap::Heap(const Space& space, const MemoryRegion& region):
    space_(space),
    region_(region),
    from_(this, GetFromSemispaceStart(region), GetSemispaceSize(region)),
    to_(this, GetToSemispaceStart(region), GetSemispaceSize(region)){
    region_.Protect(MemoryRegion::kReadWrite);
  }

  RawObject* Semispace::AllocateRawObject(const uint64_t& size){
    uint64_t total_size = sizeof(RawObject) + size;
    if(!Contains(current_ + total_size))
      return nullptr;//TODO: validate check or scavenge?
    DLOG(INFO) << "allocating object of " << HumanReadableSize(size) << " (total_size=" << HumanReadableSize(total_size) << ") @" << (void*)current_;
    uword paddress = current_;
    current_ += total_size;
    void* ptr = (void*)paddress;
    memset(ptr, 0, total_size);
    auto raw = new (ptr)RawObject();
    raw->SetColor(Color::kWhite);
    raw->SetGenerations(0);
    raw->SetReferences(0);
    raw->SetSpace(GetHeap()->GetSpace());
    raw->SetPointerSize(size);
    raw->SetPointerAddress(paddress + sizeof(RawObject));
    return raw;
  }

  void* Semispace::Allocate(const uint64_t& size){
    return AllocateRawObject(size)->GetPointer();
  }

  void Semispace::VisitMarkedRawObjectPointers(RawObjectPointerVisitor* vis){
    SemispaceIterator iter(this);
    while(iter.HasNext()){
      auto obj = iter.Next();
      if(obj->GetColor() == Color::kMarked && !vis->Visit(obj))
        return;
    }
  }

  void Semispace::VisitRawObjectPointers(RawObjectPointerVisitor* vis){
    SemispaceIterator iter(this);
    while(iter.HasNext()){
      auto obj = iter.Next();
      if(!vis->Visit(obj))
        return;
    }
  }

  void Semispace::VisitObjectPointers(ObjectPointerVisitor* vis){
    SemispaceIterator iter(this);
    while(iter.HasNext()){
      auto obj = iter.Next();
      if(!vis->Visit((Object*)obj->GetPointer()))//TODO: Replace w/ GetObjectPointer
        return;
    }
  }

  void Heap::VisitMarkedRawObjectPointers(RawObjectPointerVisitor* vis){
    HeapIterator iter(this);
    while(iter.HasNext()){
      auto obj = iter.Next();
      if(obj->GetColor() == Color::kMarked && !vis->Visit(obj))
        return;
    }
  }

  void Heap::VisitRawObjectPointers(RawObjectPointerVisitor* vis){
    HeapIterator iter(this);
    while(iter.HasNext()){
      auto obj = iter.Next();
      if(!vis->Visit(obj))
        return;
    }
  }

  void Heap::VisitObjectPointers(ObjectPointerVisitor* vis){
    HeapIterator iter(this);
    while(iter.HasNext()){
      auto obj = iter.Next();
      if(!vis->Visit((Object*)obj->GetPointer()))//TODO: Replace w/ GetObjectPointer
        return;
    }
  }

  void Heap::SwapSpaces(){
    std::swap(from_, to_);
  }
}