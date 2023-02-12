#include "poseidon/type.h"
#include "freelist.h"
#include "freelist_printer.h"

namespace poseidon{
#define ITERATE_FREE_POINTERS_BEGIN \
 FreeListIterator iter(this);       \
 while(iter.HasNext()) {            \
  auto next = iter.Next();

#define ITERATE_FREE_POINTERS_END \
 }                                \
 return true;

#define PSDN_CANNOT_INSERT(Level, Region, Reason) \
 DLOG(Level) << "cannot insert " << (Region) << " into freelist: " << (Reason);

#define PSDN_CANNOT_REMOVE(Level, Region, Reason) \
 DLOG(Level) << "cannot remove " << (Region) << " from " << (*this) << ": " << (Reason);

#define PSDN_CANNOT_FIND(Level, Size, Reason) \
 DLOG(Level) << "cannot find " << Bytes((Size)) << " in freelist: " << (Reason);

 bool FreeList::VisitFreePointers(FreePointerVisitor* vis){
   ITERATE_FREE_POINTERS_BEGIN
     if(!vis->VisitFreePointer(next))
       return false;
   ITERATE_FREE_POINTERS_END;
 }

 bool FreeList::VisitFreePointers(const std::function<bool(FreePointer*)>& vis){
   ITERATE_FREE_POINTERS_BEGIN
     if(!vis(next))
       return false;
   ITERATE_FREE_POINTERS_END
 }

 bool FreeList::VisitPointers(RawObjectVisitor* vis){
   NOT_IMPLEMENTED(FATAL);
   return false;
 }

 bool FreeList::VisitMarkedPointers(RawObjectVisitor* vis){
   NOT_IMPLEMENTED(FATAL);
   return false;
 }

 Pointer* FreeList::TryAllocatePointer(const ObjectSize size){
   if(size <= 0 || size > flags::GetOldZoneSize())
     return UNALLOCATED;

   auto total_size = size + static_cast<ObjectSize>(sizeof(Pointer));
   FreePointer* free_ptr = nullptr;
   if(!FindBestFit(size, &free_ptr)){
#ifdef PSDN_DEBUG
     LOG(ERROR) << "cannot find free list item for " << Bytes(total_size);
     FreeListPrinter::Print(this);
#endif //PSDN_DEBUG
     CannotAllocate(this, size);
   }

   if(!Remove(free_ptr)){
     DLOG(ERROR) << "cannot remove " << (*free_ptr) << " from " << (*this);
     return UNALLOCATED;
   }

   if(CanSplitAfter(free_ptr, total_size)){
     if(!SplitAfterAndInsert(free_ptr, total_size)){
       DLOG(ERROR) << "cannot split " << (*free_ptr) << " in " << (*this);
       return UNALLOCATED; //TODO: add ptr back to freelist
     }
   }
   return new(free_ptr->GetStartingAddressPointer()) Pointer(PointerTag::Old(size));
 }

 uword FreeList::TryAllocateBytes(const ObjectSize size){
   auto new_ptr = TryAllocatePointer(size);
   if(new_ptr == UNALLOCATED)
     return UNALLOCATED;
   return new_ptr->GetObjectPointerAddress();
 }

 uword FreeList::TryAllocateClassBytes(Class* cls){
   return TryAllocateBytes(cls->GetAllocationSize());
 }

 static inline bool
 CanFitIn(const Region& region, const ObjectSize size){
   return region.GetSize() == size ||
       (region.GetSize() - size) >= FreeList::GetMinimumSize();
 }

 static inline bool
 FindFirstFitIn(FreePointer** list, const ObjectSize size, FreePointer** result){
   auto item = (*list);
   while(item != nullptr){
     if(CanFitIn((Region) *item, size)){
       (*result) = item;
       return true;
     }
     item = item->GetNext();
   }
   (*result) = nullptr;
   return false;
 }

 bool FreeList::FindBestFit(const ObjectSize size, FreePointer** result){
   if(size < GetMinimumSize() || size > GetMaximumSize()){
     PSDN_CANNOT_FIND(ERROR, size, "invalid size");
     return false;
   }
   const auto bucket = GetBucketIndexFor(size);
   DLOG(INFO) << "getting " << Bytes(size) << " from list #" << bucket << " in " << (*this) << "....";
   if(FindFirstFitIn(&buckets_[bucket], size, result))
     return true;
   PSDN_CANNOT_FIND(WARNING, size, "cannot find match");
   return false;
 }

 bool FreeList::Remove(const Region region){
   if(GetMinimumSize() > region.GetSize() || region.GetSize() > GetSize()){
     PSDN_CANNOT_REMOVE(WARNING, region, "invalid size");
     return false;
   }

   DLOG(INFO) << "removing " << region << " from " << (*this);
   auto bucket = GetBucketIndexFor(region.GetSize());
   auto entry = buckets_[bucket];
   FreePointer* previous = nullptr;
   while(entry != nullptr && entry->GetSize() > region.GetSize()
       && entry->GetStartingAddress() != region.GetStartingAddress()){
     previous = entry;
     entry = entry->GetNext();
   }

   if(entry == nullptr)
     return false;

   auto next = entry->GetNext();
   if(previous == nullptr){
     buckets_[bucket] = next;
   } else{
     previous->SetNextAddress(next->GetAddress());
   }
   return true;
 }

 void FreeList::ClearFreeList(){
   DLOG(INFO) << "clearing " << (*this) << "....";
   Insert((const Region&) *this);
 }

 bool FreeList::Insert(const Region region){
   if(region.GetSize() < GetMinimumSize() || region.GetSize() > GetSize()){
     PSDN_CANNOT_INSERT(WARNING, region, "invalid size");
     return false;
   }

   InsertTail(&head_, region);
   return true;
 }
}