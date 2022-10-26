#include "poseidon/heap/freelist.h"

namespace poseidon{
#define UNALLOCATED 0 //TODO: cleanup

 uword FreeList::TryAllocate(int64_t size){
   NOT_IMPLEMENTED(ERROR); //TODO: implement
   return UNALLOCATED;
 }

 bool FreeList::Visit(poseidon::FreeListNodeVisitor* vis){
   FreeListIterator iter(this);
   while(iter.HasNext()) {
     auto next = iter.Next();
     if(!vis->Visit(next))
       return false;
   }
   return true;
 }
}