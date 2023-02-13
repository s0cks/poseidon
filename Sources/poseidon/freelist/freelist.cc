#include "poseidon/object.h"
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

 void FreeList::InsertAtHead(const Region& region) {
   if(IsEmpty()) {
     head_ = tail_ = FreePointer::From(region);
     num_nodes_ += 1;
   } else {
     auto node = FreePointer::From(region);
     node->SetNextAddress(GetHead()->GetAddress());
     head_ = node;
     num_nodes_ += 1;
   }
 }

 void FreeList::InsertAtTail(const Region& region) {
   if(IsEmpty()) {
     head_ = tail_ = FreePointer::From(region);
     num_nodes_ += 1;
   } else {
     auto new_node = FreePointer::From(region);
     tail_->SetNextAddress(new_node->GetAddress());
     tail_ = new_node;
     num_nodes_ += 1;
   }
 }

 Pointer* FreeList::TryAllocatePointer(const ObjectSize size){
   if(size <= 0 || size > flags::GetOldZoneSize())
     return UNALLOCATED;

   auto total_size = size + static_cast<ObjectSize>(sizeof(Pointer));
   FreePointer* free_ptr = FindBestFit(size);
   if(free_ptr == UNALLOCATED) {
     //TODO: handle
     return nullptr;
   }

   Remove((const Region&)*free_ptr);

   if(CanSplitAfter(free_ptr, total_size))
     SplitAfterAndInsert(free_ptr, total_size);
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

 void FreeList::Remove(const Region& region){
   if(GetMinimumSize() > region.GetSize() || region.GetSize() > GetSize()){
     PSDN_CANNOT_REMOVE(WARNING, region, "invalid size");
     return;
   }
   DLOG(INFO) << "removing " << region << " from " << (*this);
 }

 void FreeList::ClearFreeList(){
   DLOG(INFO) << "clearing " << (*this) << "....";
   Insert((const Region&) *this);
 }

 FreePointer* FreeList::FindBestFit(ObjectSize size) {
   NOT_IMPLEMENTED(FATAL);
   return nullptr;
 }

 void FreeList::Insert(const Region& region){
   if(region.GetSize() < GetMinimumSize() || region.GetSize() > GetSize()){
     PSDN_CANNOT_INSERT(WARNING, region, "invalid size");
     return;
   }
   DLOG(INFO) << "inserting " << region << " into " << (*this);

   if(IsEmpty() || GetHead()->GetSize() > region.GetSize()){
     return InsertAtHead(region);
   }

   auto node = GetHead();
   while(node->GetSize() < region.GetSize() && node->HasNext()) {
     if(node->GetNext()->GetSize() >= region.GetSize()) {
       auto new_node = FreePointer::From(region);
       new_node->SetNextAddress(node->GetNextAddress());
       node->SetNextAddress(new_node->GetAddress());
       break;
     }

     node = node->GetNext();
   }
 }
}