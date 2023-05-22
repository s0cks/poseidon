#ifndef POSEIDON_FREELIST_H
#define POSEIDON_FREELIST_H

#include <cstdlib>
#include <utility>
#include <glog/logging.h>

#include "poseidon/flags.h"
#include "poseidon/utils.h"
#include "poseidon/bitset.h"
#include "poseidon/pointer.h"
#include "poseidon/heap/section.h"
#include "poseidon/platform/platform.h"
#include "poseidon/freelist/free_ptr.h"

namespace poseidon{ //TODO: atomic support?
 class Class;
 class FreeList : public Section {
   friend class Sweeper;
   friend class OldZone;

   friend class FreeListTest;
   friend class SerialSweeperTest;
  public:
   static constexpr ObjectSize
   GetMinimumSize() {
     return kWordSize;
   }

   static ObjectSize
   GetMaximumSize() {
     return flags::GetLargeObjectSize();
   }

   class FreeListIterator {
    protected:
     const FreeList* free_list_;
     uword current_;

     inline const FreeList* free_list() const {
       return free_list_;
     }

     inline uword current_address() const {
       return current_;
     }

     inline RawPointerTag current_raw_tag() const {
       return (RawPointerTag)current_address();
     }

     inline PointerTag current_tag() const {
       return (PointerTag)current_raw_tag();
     }
    public:
     explicit FreeListIterator(const FreeList* free_list):
      free_list_(free_list),
      current_(free_list->GetStartingAddress()) {
     }
     ~FreeListIterator() = default;

     bool HasNext() {
       const auto tag = current_tag();
       DLOG(INFO) << "current tag @" << (void*)current_address() << ": " << tag;
       return current_address() < free_list()->GetEndingAddress();
     }

     FreePointer* Next() {
       NOT_IMPLEMENTED(FATAL);
       return nullptr;
     }
   };
  protected:
   FreePointer* head_;
   FreePointer* tail_;
   int64_t num_nodes_;

   static inline int GetBucketIndexFor(const ObjectSize& size){
     return static_cast<int>((size << kWordSize) % flags::GetNumberOfFreeListBuckets());
   }

   static inline void
   InsertAfter(FreePointer* before, const Region& region) {
     PSDN_ASSERT(before != nullptr);
     DLOG(INFO) << "inserting " << region << " after " << (*before);
     auto new_node = FreePointer::From(region);
     new_node->SetNextAddress(before->GetNextAddress());
     before->SetNextAddress(new_node->GetStartingAddress());
   }

   inline void MergeAndInsertAfter(FreePointer* lhs, const Region& region) {
     PSDN_ASSERT(CanMerge(lhs, region));
     auto new_start = lhs->GetStartingAddress();
     auto new_size = lhs->GetSize() + region.GetSize();
     return Insert({ new_start, new_size });
   }

   static inline bool
   CanSplitAfter(const FreePointer* ptr, const ObjectSize& size) {
     return ptr->GetSize() - (size + sizeof(Pointer)) > kWordSize; //TODO: cleanup
   }

   static inline bool
   CanMerge(const FreePointer* lhs, const Region& region) {
     return lhs->GetEndingAddress() == region.GetStartingAddress(); // ???
   }

   inline void SplitAfterAndInsert(FreePointer* parent, const ObjectSize& size){
     DLOG(INFO) << "splitting " << Bytes(size) << " from " << (*parent);
     // we need to split from the end
     const auto next_address = parent->GetStartingAddress() + size;
     const auto new_size = parent->GetSize() - size;
     const auto new_ptr = new ((void*) next_address)FreePointer(PointerTag::Old(new_size));
     return Insert({ new_ptr->GetStartingAddress(), new_ptr->GetSize() });
   }

   virtual void ClearFreeList();
   virtual void Remove(const Region& region);
   virtual void Insert(const Region& region);
   virtual FreePointer* FindBestFit(ObjectSize size);

   inline FreePointer* GetHead() const {
     return head_;
   }

   inline bool IsEmpty() const {
     return GetHead() == nullptr;
   }
  public: //TODO: visible for testing
   void InsertAtHead(const Region& region);
   void InsertAtTail(const Region& region);
  public:
   FreeList() = default;
   FreeList(const uword start, const RegionSize size, const bool insert_region = true):
    Section(start, size),
      head_(nullptr),
      num_nodes_(0) {
     if(insert_region)
       InsertAtHead({ start, size });
   }
   explicit FreeList(const MemoryRegion& region, const bool insert_region = true):
    FreeList(region.GetStartingAddress(), region.GetSize(), insert_region) {
   }
   FreeList(const FreeList& rhs) = default;
   ~FreeList() override = default;

   int64_t GetNumberOfNodes() const {
     return num_nodes_;
   }

   virtual Pointer* TryAllocatePointer(ObjectSize size);
   virtual uword TryAllocateBytes(ObjectSize size);
   virtual uword TryAllocateClassBytes(Class* cls);

   template<typename T>
   T* TryAllocate() {
     return TryAllocateBytes(sizeof(T));
   }

   template<class T>
   T* TryAllocateClass() {
     return TryAllocateClassBytes(T::GetClass());
   }

   virtual inline bool Contains(FreePointer* free_ptr) const {
     return Region::Contains((Region) *free_ptr);
   }

   virtual inline bool Intersects(FreePointer* free_ptr) const {
     return Region::Intersects((Region) *free_ptr);
   }

   bool VisitPointers(RawObjectVisitor* vis) const override;
   bool VisitMarkedPointers(RawObjectVisitor* vis) const override;

   bool VisitUnmarkedPointers(RawObjectVisitor* vis) const override {
     NOT_IMPLEMENTED(FATAL); //TODO: implement
     return false;
   }

   bool VisitNewPointers(RawObjectVisitor* vis) const override {
     NOT_IMPLEMENTED(ERROR); //TODO: implement
     return false;
   }

   bool VisitOldPointers(RawObjectVisitor* vis) const override {
     NOT_IMPLEMENTED(ERROR); //TODO: implement
     return false;
   }

   virtual bool VisitFreePointers(FreePointerVisitor* vis) const;
   virtual bool VisitFreePointers(const std::function<bool(FreePointer*)>& vis) const;

   friend std::ostream& operator<<(std::ostream& stream, const FreeList& value) {
     stream << "FreeList(";
     stream << "start=" << value.GetStartingAddressPointer() << ", ";
     stream << "size=" << Bytes(value.GetSize());
     stream << ")";
     return stream;
   }
 };
}

#endif//POSEIDON_FREELIST_H