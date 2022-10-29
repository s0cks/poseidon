#include "poseidon/heap/freelist.h"

namespace poseidon{
#define UNALLOCATED 0 //TODO: cleanup

 static inline bool
 CanSplit(const FreeObject* ptr, const ObjectSize& size) {
   return ptr->GetSize() - size > kWordSize;
 }

 static FreeObject* Split(FreeObject* parent, const ObjectSize& size){
   // we need to split from the end
   const auto next_address = parent->GetStartingAddress() + size;
   const auto new_size = parent->GetSize() - size;
   return new((void*) next_address)FreeObject(ObjectTag::Old(new_size));
 }

 uword FreeList::TryAllocate(const ObjectSize& size) {
   if(size <= 0 || size > GetOldZoneSize())
     return UNALLOCATED;
   NOT_IMPLEMENTED(FATAL); //TODO: implement
   return UNALLOCATED;
 }

 FreeObject* FreeList::FindBestFit(const ObjectSize& size) {
   if(size <= 0 || size > GetOldPageSize())
     return UNALLOCATED;
   NOT_IMPLEMENTED(FATAL);//TODO: implement
   return UNALLOCATED;
 }

 bool FreeList::Remove(const uword start, const ObjectSize size){
   auto bucket = GetBucketIndexFor(size);
   DLOG(INFO) << Bytes(size) << "bucket: " << bucket;

   auto entry = buckets_[bucket];
   FreeObject* previous = nullptr;
   while(entry != nullptr && entry->GetStartingAddress() != start) {
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

 void FreeList::Insert(const uword start, const ObjectSize size){
   auto bucket = GetBucketIndexFor(size);
   DLOG(INFO) << "bucket: " << bucket;

   auto entry = buckets_[bucket];
   if (entry == nullptr) {
     buckets_[bucket] = FreeObject::From(MemoryRegion(start, size));
     return;
   }

   FreeObject* previous = nullptr;
   while(entry != nullptr && entry->GetSize() < size) { // find the first size equal to
     previous = entry;
     entry = entry->GetNext();
   }

   while(entry != nullptr && entry->GetAddress() < start) { // find the first with an address greater than
     previous = entry;
     entry = entry->GetNext();
   }

   if(previous != nullptr)
     InsertAfter(previous, start, size);
 }
}