#ifndef POSEIDON_FREELIST_H
#define POSEIDON_FREELIST_H

#include <cstdlib>
#include <utility>
#include <glog/logging.h>

#include "poseidon/flags.h"
#include "poseidon/utils.h"
#include "poseidon/bitset.h"
#include "poseidon/pointer.h"
#include "poseidon/platform/platform.h"
#include "poseidon/heap/section.h"
#include "poseidon/heap/free_object.h"

namespace poseidon{ //TODO: atomic support?
 class Class;
 class FreeList {
   friend class Sweeper;
   friend class OldZone;

   friend class FreeListTest;
   friend class SerialSweeperTest;
  public:
   class FreeListIterator {
    protected:
     const FreeList* free_list_;
     uword current_;

     const FreeList* free_list() const {
       return free_list_;
     }

     inline uword current_address() const {
       return current_;
     }

     inline FreeObject* current_ptr() const {
       return (FreeObject*)current_address();
     }
    public:
     explicit FreeListIterator(const FreeList* free_list):
      free_list_(free_list),
      current_(free_list->GetStartingAddress()){
     }
     ~FreeListIterator() = default;

     bool HasNext() const {
       return current_address() < free_list()->GetEndingAddress();
     }

     FreeObject* Next() {
       auto next = current_ptr();
       do {
         current_ += next->GetSize();
       } while(HasNext() && !current_ptr()->IsFree());
       return next;
     }
   };
  protected:
   uword start_;
   int64_t total_size_;
   FreeObject** buckets_;
   int64_t num_nodes_;

   static inline int GetBucketIndexFor(const ObjectSize& size){
     return static_cast<int>(size % flags::GetNumberOfFreeListBuckets());
   }

   static inline bool
   InsertAfter(FreeObject* before, FreeObject* node) {
     if(before == nullptr)
       return false;
     DLOG(INFO) << "inserting " << (*node) << " after " << (*before);
     node->SetNextAddress(before->GetNextAddress());
     before->SetNextAddress(node->GetAddress());
     return true;
   }

   inline bool MergeAndInsertAfter(FreeObject* lhs, FreeObject* rhs) {
     PSDN_ASSERT(CanMerge(lhs, rhs));
     auto new_start = lhs->GetStartingAddress();
     auto new_size = lhs->GetSize() + rhs->GetSize();
     return Insert(new_start, new_size);
   }

   static inline bool
   CanSplitAfter(const FreeObject* ptr, const ObjectSize& size) {
     return ptr->GetSize() - (size + sizeof(Pointer)) > kWordSize; //TODO: cleanup
   }

   static inline bool
   CanMerge(const FreeObject* lhs, const FreeObject* rhs) {
     return lhs->GetEndingAddress() == rhs->GetStartingAddress(); // ???
   }

   inline bool SplitAfterAndInsert(FreeObject* parent, const ObjectSize& size){
     // we need to split from the end
     const auto next_address = parent->GetStartingAddress() + size;
     const auto new_size = parent->GetSize() - size;
     const auto new_ptr = new ((void*) next_address)FreeObject(PointerTag::Old(new_size));
     return Insert(new_ptr->GetStartingAddress(), new_ptr->GetSize());
   }
  public:
   FreeList() = default;
   FreeList(const uword start, const ObjectSize size):
     buckets_(new FreeObject*[flags::GetNumberOfFreeListBuckets()]),
     num_nodes_(0),
     start_(start),
     total_size_(size) {
     for(auto idx = 0; idx < flags::GetNumberOfFreeListBuckets(); idx++)
       buckets_[idx] = nullptr;
   }
   explicit FreeList(const MemoryRegion& region):
    FreeList(region.GetStartingAddress(), region.GetSize()) {
   }
   FreeList(const FreeList& rhs) = default;
   ~FreeList() = default;

   uword GetStartingAddress() const {
     return start_;
   }

   void* GetStartingAddressPointer() const {
     return (void*)GetStartingAddress();
   }

   uword GetEndingAddress() const {
     return GetStartingAddress() + GetSize();
   }

   void* GetEndingAddressPointer() const {
     return (void*)GetEndingAddress();
   }

   int64_t GetSize() const {
     return total_size_;
   }

   int64_t GetNumberOfNodes() const {
     return num_nodes_;
   }

   virtual FreeObject* FindFirstFit(ObjectSize size); //TODO: change access?

   virtual inline bool Contains(const uword address) const {
     return address >= GetStartingAddress() &&
            address <= GetEndingAddress();
   }

   virtual bool Contains(const uword start_address, const ObjectSize size) const {
     const auto end_address = start_address + size;
     return Contains(start_address) && Contains(end_address);
   }

   inline bool Contains(Pointer* raw_ptr) const {
     return Contains(raw_ptr->GetStartingAddress(), raw_ptr->GetSize());
   }

   inline bool Contains(FreeObject* free_ptr) const {
     return Contains(free_ptr->GetStartingAddress(), free_ptr->GetSize());
   }

   inline bool IntersectedBy(const uword start_address, const ObjectSize size) const {
     const auto end_address = start_address + size;
     return Contains(start_address) || Contains(end_address);
   }

   inline bool IntersectedBy(const Pointer* ptr) const {
     return IntersectedBy(ptr->GetStartingAddress(), ptr->GetTotalSize());
   }

   inline bool IntersectedBy(const FreeObject* ptr) const {
     return IntersectedBy(ptr->GetStartingAddress(), ptr->GetTotalSize());
   }

   virtual bool Insert(uword start, ObjectSize size);

   inline bool Insert(Pointer* raw_ptr) {
     return Insert(raw_ptr->GetStartingAddress(), raw_ptr->GetTotalSize());
   }

   inline bool Insert(FreeObject* free_ptr) {
     return Insert(free_ptr->GetStartingAddress(), free_ptr->GetSize());
   }

   virtual bool Remove(uword start, ObjectSize size);

   inline bool Remove(FreeObject* free_ptr) {
     return Remove(free_ptr->GetStartingAddress(), free_ptr->GetSize());
   }

   virtual Pointer* TryAllocatePointer(word size);
   virtual uword TryAllocateBytes(word size);
   virtual uword TryAllocateClassBytes(Class* cls);

   template<typename T>
   T* TryAllocate() {
     return TryAllocateBytes(sizeof(T));
   }

   template<class T>
   T* TryAllocateClass() {
     return TryAllocateClassBytes(T::GetClass());
   }

   virtual bool VisitFreePointers(FreeObjectVisitor* vis);

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