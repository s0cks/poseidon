#ifndef POSEIDON_HELPERS_H
#define POSEIDON_HELPERS_H

#include <gtest/gtest.h>
#include <glog/logging.h>

#include "poseidon/heap.h"
#include "poseidon/object.h"

namespace poseidon{
 template<class T>
 static inline T*
 New(HeapPage* page, Class* cls){
   auto data = page->Allocate(cls->GetAllocationSize());

   auto fake = new Instance(cls);
   fake->set_raw(data);
   memcpy(data->GetPointer(), fake, sizeof(Instance));
   delete fake;

   return (T*)data->GetPointer();
 }

 static inline Int*
 NewInt(HeapPage* page, uint64_t val){
   auto value = New<Int>(page, Class::CLASS_INT);
   value->Set(val);
   return value;
 }

 static inline Local<Int>
 NewLocalInt(HeapPage* page, uint64_t val){
   auto local = Allocator::AllocateLocal<Int>();
   local.Set(NewInt(page, val));
   return local;
 }
}

#endif //POSEIDON_HELPERS_H
