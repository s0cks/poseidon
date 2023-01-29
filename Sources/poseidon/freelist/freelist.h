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
     int64_t bucket_;
     FreePointer* current_;

     inline const FreeList* free_list() const {
       return free_list_;
     }

     inline int64_t current_bucket_index() const {
       return bucket_;
     }

     inline int64_t num_buckets() const {
       return free_list()->num_buckets_;
     }

     inline FreePointer* current_ptr() const {
       return current_;
     }
    public:
     explicit FreeListIterator(const FreeList* free_list):
      free_list_(free_list),
      bucket_(0),
      current_(free_list->buckets_[current_bucket_index()]) {
     }
     ~FreeListIterator() = default;

     bool HasNext() {
       auto current = current_ptr();
       while(current == nullptr && current_bucket_index() <= num_buckets()) {
         bucket_ += 1;
         current_ = current = free_list()->buckets_[current_bucket_index()];
       }
       return current_ptr() != nullptr ||
              current_bucket_index() <= num_buckets();
     }

     FreePointer* Next() {
       auto next = current_ptr();
       current_ = next->GetNext();
       return next;
     }
   };
  protected:
   FreePointer** buckets_;
   int64_t num_buckets_;
   int64_t num_nodes_;

   static inline int GetBucketIndexFor(const ObjectSize& size){
     return static_cast<int>((size << kWordSize) % flags::GetNumberOfFreeListBuckets());
   }

   static inline void
   InsertHead(FreePointer** list, const Region& region) {
     auto free_ptr = FreePointer::From(region);
     if(free_ptr == nullptr)
       return;
     free_ptr->SetNext((*list));
     (*list) = free_ptr;
   }

   static inline void
   InsertTail(FreePointer** list, const Region& region) {
     if((*list) == nullptr)
       return InsertHead(list, region);
     auto last = (*list);
     while(last->HasNext())
       last = last->GetNext();
     return InsertAfter(last, region);
   }

   static inline void
   InsertAfter(FreePointer* before, const Region& region) {
     PSDN_ASSERT(before != nullptr);
     DLOG(INFO) << "inserting " << region << " after " << (*before);
     auto new_node = FreePointer::From(region);
     new_node->SetNextAddress(before->GetNextAddress());
     before->SetNextAddress(new_node->GetStartingAddress());
   }

   inline bool MergeAndInsertAfter(FreePointer* lhs, const Region& region) {
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

   inline bool SplitAfterAndInsert(FreePointer* parent, const ObjectSize& size){
     DLOG(INFO) << "splitting " << Bytes(size) << " from " << (*parent);
     // we need to split from the end
     const auto next_address = parent->GetStartingAddress() + size;
     const auto new_size = parent->GetSize() - size;
     const auto new_ptr = new ((void*) next_address)FreePointer(PointerTag::Old(new_size));
     return Insert({ new_ptr->GetStartingAddress(), new_ptr->GetSize() });
   }

   virtual void ClearFreeList();
   virtual bool Remove(Region region);
   virtual bool Insert(Region region);
   virtual bool InsertHead(const Region& region);
   virtual bool FindBestFit(ObjectSize size, FreePointer** result);
  public:
   FreeList() = default;
   FreeList(const uword start, const RegionSize size, const bool insert_region = true, const word num_buckets = flags::GetNumberOfFreeListBuckets()):
    Section(start, size),
     buckets_(),
     num_buckets_(),
     num_nodes_(0) {
     if(num_buckets > 0) {
       const auto nbuckets = static_cast<word>(RoundUpPowTwo(num_buckets));
       DLOG(INFO) << "initializing FreeList with " << nbuckets << " buckets....";
       auto buckets = (FreePointer**)malloc(sizeof(FreePointer*) * nbuckets);
       LOG_IF(FATAL, !buckets) << "failed to allocate bucket array for FreeList w/ " << nbuckets << " buckets";
       buckets_ = buckets;
       num_buckets_ = nbuckets;
       for(auto idx = 0; idx < nbuckets; idx++)
         buckets_[idx] = nullptr;
     }
     LOG_IF(FATAL, insert_region && !InsertHead({ start, size })) << "failed to insert region into freelist";
   }
   explicit FreeList(const MemoryRegion& region, const bool insert_region = true, const word num_buckets = flags::GetNumberOfFreeListBuckets()):
    FreeList(region.GetStartingAddress(), region.GetSize(), insert_region, num_buckets) {
   }
   FreeList(const FreeList& rhs) = default;
   ~FreeList() override = default;

   int64_t GetNumberOfNodes() const {
     return num_nodes_;
   }

   template<class P>
   inline bool Insert(P* ptr) {
     return Insert((Region) *ptr);
   }

   template<class P>
   inline bool Remove(P* ptr) {
     return Remove((Region) *ptr);
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

   bool VisitPointers(RawObjectVisitor* vis) override;
   bool VisitMarkedPointers(RawObjectVisitor* vis) override;

   bool VisitUnmarkedPointers(RawObjectVisitor* vis) override {
     NOT_IMPLEMENTED(FATAL); //TODO: implement
     return false;
   }

   bool VisitNewPointers(RawObjectVisitor* vis) override {
     NOT_IMPLEMENTED(ERROR); //TODO: implement
     return false;
   }

   bool VisitOldPointers(RawObjectVisitor* vis) override {
     NOT_IMPLEMENTED(ERROR); //TODO: implement
     return false;
   }

   virtual bool VisitFreePointers(FreePointerVisitor* vis);
   virtual bool VisitFreePointers(const std::function<bool(FreePointer*)>& vis);

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