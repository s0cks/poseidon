#ifndef POSEIDON_HELPERS_H
#define POSEIDON_HELPERS_H

#include <gtest/gtest.h>
#include <glog/logging.h>

#include "poseidon/local.h"
#include "poseidon/heap/heap.h"

namespace poseidon{
 using namespace ::testing;

 static inline uword
 FailAllocation(int64_t size){
   DLOG(ERROR) << "allocating new object (" << Bytes(size) << ") failed.";
   return 0;
 }

 static inline uword
 SystemAlloc(int64_t size){
   return (uword)malloc(size);
 }

 static inline uword
 SystemAllocNewObject(int64_t size){
   DLOG(INFO) << "allocating new object (" << Bytes(size) << ") using malloc.";
   auto total_size = sizeof(RawObject) + size;
   auto val = (RawObject*)SystemAlloc(size);
   if(!val){
     LOG(FATAL) << "failed to allocate new object (" << Bytes(size) << ") using system [malloc].";
     return 0;
   }

   memset((void*)val, 0, total_size);
   val->SetPointerSize(size);
   return val->GetAddress();
 }

 static inline AssertionResult
 IsAllocated(RawObject* val){
   if(!val)
     return AssertionFailure() << "Expected to be allocated.";
   return AssertionSuccess();
 }

 static inline AssertionResult
 IsNew(RawObject* val){
   if(!val->IsNew())
     return AssertionFailure() << "Expected " << val->ToString() << " to be new.";
   return AssertionSuccess();
 }

 template<typename T>
 static inline AssertionResult
 IsNew(const Local<T>& local){
   return IsNew(local.raw());
 }

 static inline AssertionResult
 IsOld(RawObject* val){
   if(!val->IsOld())
     return AssertionFailure() << "Expected " << val->ToString() << " to be old.";
   return AssertionSuccess();
 }

 template<typename T>
 static inline AssertionResult
 IsOld(const Local<T>& val){
   return IsOld(val.raw());
 }

 static inline AssertionResult
 IsMarked(RawObject* val){
   if(!val->IsMarked())
     return AssertionFailure() << "Expected " << val->ToString() << " to marked.";
   return AssertionSuccess();
 }

 template<typename T>
 static inline AssertionResult
 IsMarked(const Local<T>& val){
   return IsMarked(val.raw());
 }

 static inline AssertionResult
 IsRemembered(RawObject* val){
   if(!val->IsRemembered())
     return AssertionFailure() << "Expected " << val->ToString() << " to be remembered.";
   return AssertionSuccess();
 }

 template<typename T>
 static inline AssertionResult
 IsRemembered(const Local<T>& val){
   return IsRemembered(val.raw());
 }

 static inline AssertionResult
 IsForwarding(RawObject* val){
   if(!val->IsForwarding())
     return AssertionFailure() << "Expected " << val->ToString() << " to be forwarding.";
   return AssertionSuccess();
 }

 template<typename T>
 static inline AssertionResult
 IsForwarding(const Local<T>& val){
   return IsForwarding(val.raw());
 }

 template<class S>
 static inline RawObject*
 TryAllocateNewWord(S* section){
   return (RawObject*)section->TryAllocate(kWordSize);
 }

 template<class S>
 static inline RawObject*
 TryAllocateNewWord(S* section, word value){
   auto val = TryAllocateNewWord(section);
   if(val != nullptr)
     *((word*)val->GetPointer()) = value;
   return val;
 }

 template<class S>
 static inline RawObject*
 TryAllocateNewMarkedWord(S* section, word value = 0){
   auto val = TryAllocateNewWord(section, value);
   if(val != nullptr)
     val->SetMarkedBit();
   return val;
 }

 template<class S>
 static inline RawObject*
 TryAllocateNewRememberedWord(S* section, word value = 0){
   auto val = TryAllocateNewWord(section, value);
   if(val != nullptr)
     val->SetRememberedBit();
   return val;
 }

 static inline AssertionResult
 WordIs(RawObject* ptr, word value){
   if(ptr->GetPointerSize() != kWordSize)
     return AssertionFailure() << ptr->ToString() << " is not a word.";
   auto lhs = *((word*)ptr->GetPointer());
   if(lhs != value)
     return AssertionFailure() << ptr->ToString() << " (" << lhs << ") does not equal: " << value;
   return AssertionSuccess();
 }
}

#endif //POSEIDON_HELPERS_H
