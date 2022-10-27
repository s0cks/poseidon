#include <cstdlib>
#include "poseidon/heap/freelist.h"

namespace poseidon{
#define UNALLOCATED 0 //TODO: cleanup

 bool FreeList::Remove(const uword starting_address, const ObjectSize size) {
   auto node = GetHead();
   FreeObject* previous = nullptr;
   if(node != nullptr && (node->GetStartingAddress() == starting_address && node->GetSize() == size)) {
     if(node->GetNextAddress() >= GetEndingAddress()) {
       SetHead(nullptr);
     } else{
       SetHead(node->GetNext());
     }
     num_nodes_ -= 1;
     return true;
   }

   while(node != nullptr && !(node->GetStartingAddress() == starting_address && node->GetSize() == size)) {
     previous = node;
     node = node->GetNext();
   }

   if(node == nullptr)
     return false;

   num_nodes_ -= 1;
   previous->SetNextAddress(node->GetNextAddress());
   return true;
 }

 bool FreeList::Insert(const uword starting_address, const ObjectSize size) {
   if(size <= 0 || size > GetSize()) {
     DLOG(WARNING) << "cannot insert @" << ((void*)starting_address) << " (" << Bytes(size) << ") into FreeList";
     return false;
   }

   auto new_node = new ((void*) starting_address)FreeObject(size);
   if(IsEmpty() || GetHead()->GetSize() < size) {
     return InsertAtHead(new_node);
   }

   FreeObject* node = GetHead();
   while(node != nullptr && node->GetSize() > 0 && node->GetNextAddress() < GetEndingAddress()) {
     if(node->GetSize() > new_node->GetSize())
       break;
     node = node->GetNext();
   }

   DLOG(INFO) << "inserting " << (*new_node) << " after: " << (*node);
   num_nodes_ += 1;
   new_node->SetNextAddress(node->GetNextAddress());
   node->SetNextAddress(new_node->GetAddress());
   return true;
 }

 bool FreeList::InsertAtHead(FreeObject* node){
   DLOG(INFO) << "inserting " << (*node) << " at head";
   if(IsEmpty()) {
     SetHead(node);
   } else {
     node->SetNext(GetHead());
     SetHead(node);
   }
   num_nodes_ += 1;
   return true;
 }

 FreeObject* FreeList::FindBestFit(ObjectSize size){
   auto node = GetHead();
   while(node != nullptr && node->GetAddress() < GetEndingAddress()) {
     if(node->GetSize() >= size)
       return node;
     node = node->GetNext();
   }
   return nullptr;
 }

 FreeObject* FreeList::Split(FreeObject* parent, ObjectSize size){
   // we need to split from the end
   auto next_address = parent->GetStartingAddress() + size;
   auto new_size = parent->GetSize() - size;
   if(new_size <= 0)
     return nullptr;
   memset((void*) next_address, 0, new_size);
   return new ((void*)next_address)FreeObject(new_size);
 }

 uword FreeList::TryAllocate(int64_t size){
   if(size <= 0 || (size + sizeof(RawObject)) > GetSize()) {
     DLOG(WARNING) << "cannot allocate " << Bytes(size) << " in " << (*this);
     return UNALLOCATED;
   }

   auto total_size = static_cast<ObjectSize>(sizeof(RawObject)) + size;
   auto best_fit = FindBestFit(total_size);
   if(!best_fit) {
     DLOG(ERROR) << "cannot find best fit for " << Bytes(total_size) << (*this);
     return UNALLOCATED;
   }
   DLOG(INFO) << "best-fit: " << (*best_fit);
   if(!Remove(best_fit->GetStartingAddress(), best_fit->GetSize())) {
     LOG(ERROR) << "failed to remove best-fit: " << (*best_fit) << " from " << (*this);
     return UNALLOCATED;
   }

   LOG(INFO) << "allocating " << Bytes(total_size) << " in " << (*this);
   auto new_node = Split(best_fit, total_size);
   if(new_node && !Insert(new_node)) {
     DLOG(ERROR) << "cannot insert new-node: " << (*new_node);
     return UNALLOCATED;
   }

   DLOG(INFO) << "found best-fit: " << (*best_fit);
   ObjectTag tag;
   tag.SetSize(size);
   tag.ClearMarked();
   tag.SetOld();
   return (new (best_fit->GetStartingAddressPointer())RawObject(tag))->GetStartingAddress();
 }

 bool FreeList::VisitFreeNodes(poseidon::FreeListNodeVisitor* vis){
   FreeListIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(!vis->Visit(next))
       return false;
   }
   return true;
 }
}