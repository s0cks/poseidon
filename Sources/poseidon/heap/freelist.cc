#include "poseidon/heap/freelist.h"

namespace poseidon{
#define UNALLOCATED 0 //TODO: cleanup

 bool FreeList::VisitFreePointers(FreeObjectVisitor* vis){
   FreeListIterator iterator(this);
   while(iterator.HasNext()) {
     auto next = iterator.Next();
     DLOG_IF(INFO, next != nullptr) << "next: " << (*next);
     if(!vis->Visit(next))
       return false;
   }
   return true;
 }

 uword FreeList::TryAllocate(const ObjectSize& size) {
   if(size <= 0 || size > GetOldZoneSize())
     return UNALLOCATED;

   auto total_size = size + static_cast<ObjectSize>(sizeof(Pointer));
   auto free_ptr = FindFirstFit(total_size);
   if(free_ptr == UNALLOCATED) {
     DLOG(ERROR) << "cannot find free list item for " << Bytes(total_size);
     return UNALLOCATED;
   }

   if(!Remove(free_ptr)) {
     DLOG(ERROR) << "cannot remove " << (*free_ptr) << " from " << (*this);
     return UNALLOCATED;
   }

   if(CanSplitAfter(free_ptr, total_size)) {
     if(!SplitAfterAndInsert(free_ptr, total_size)) {
       DLOG(ERROR) << "cannot split " << (*free_ptr) << " in " << (*this);
       return UNALLOCATED; //TODO: add ptr back to freelist
     }
   }

   auto ptr = new (free_ptr->GetStartingAddressPointer())Pointer(PointerTag::Old(size));
   //TODO: mark all pages in old zone
   return ptr->GetStartingAddress();
 }

 FreeObject* FreeList::FindFirstFit(ObjectSize size) {
   if(size <= 0 || size > GetOldZoneSize())
     return UNALLOCATED;
   const auto bucket = GetBucketIndexFor(size);
   auto entry = buckets_[bucket];
   while(entry != nullptr) {
     if(entry->GetSize() >= size)
       return entry;
     entry = entry->GetNext();
   }

   PSDN_ASSERT(entry == nullptr);
   for(auto idx = 0; idx < GetNumberOfFreeListBuckets(); idx++) {
     entry = buckets_[idx];
     if(entry != nullptr) {
       if(entry->GetSize() >= size)
         return entry;
     }
   }

   DLOG(ERROR) << "cannot find freelist ptr for " << Bytes(size) << " in " << (*this);
   return UNALLOCATED;
 }

 bool FreeList::Remove(const uword start, const ObjectSize size){
   auto bucket = GetBucketIndexFor(size);
   auto entry = buckets_[bucket];
   FreeObject* previous = nullptr;
   while(entry != nullptr && entry->GetSize() > size && entry->GetStartingAddress() != start) {
     previous = entry;
     entry = entry->GetNext();
   }

   if(entry == nullptr)
     return false;

   auto next = entry->GetNext();
   if(previous == nullptr) {
     buckets_[bucket] = next;
   } else {
     previous->SetNextAddress(next->GetAddress());
   }
   return true;
 }

 bool FreeList::Insert(const uword start, const ObjectSize size){
   if(!Contains(start, size) || size < kWordSize)
     return false;

   auto new_ptr = FreeObject::From(MemoryRegion(start, size));
   auto bucket = GetBucketIndexFor(size);
   auto entry = buckets_[bucket];
   if (entry == nullptr) {
     buckets_[bucket] = new_ptr;
     return true;
   }

   FreeObject* previous = nullptr;
   while(entry != nullptr && entry->GetSize() > size && entry->GetStartingAddress() != start) { // find the first size equal to
     previous = entry;
     entry = entry->GetNext();
   }

   DLOG(INFO) << "entry: " << (*entry);
   DLOG_IF(INFO, previous != nullptr) << "previous: " << (*previous);

   if(entry->GetStartingAddress() == start) {
     DLOG(ERROR) << "duplicate element found: " << (*entry);
     return false;
   }

   if(previous != nullptr) {
     return InsertAfter(previous, new_ptr);
   }

   if(CanMerge(entry, new_ptr)) {
     if(!Remove(entry)) {
       DLOG(ERROR) << "cannot remove " << (*entry);
       return InsertAfter(entry, new_ptr);
     }

     return MergeAndInsertAfter(entry, new_ptr);
   }
   return InsertAfter(entry, new_ptr);
 }
}