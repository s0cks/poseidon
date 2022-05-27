#ifndef POSEIDON_ASSERTIONS_H
#define POSEIDON_ASSERTIONS_H

#include <gtest/gtest.h>

#include "poseidon/local.h"
#include "poseidon/raw_object.h"

namespace poseidon{
 using namespace ::testing;

 static inline AssertionResult
 IsAllocated(RawObject* val){
   if(!val)
     return AssertionFailure() << "Expected to be allocated.";
   return AssertionSuccess();
 }

 template<class T>
 static inline AssertionResult
 IsAllocated(const Local<T>& val){
   return IsAllocated(val.raw());
 }

 static inline AssertionResult
 IsUnallocated(RawObject* val){
   if(!val)
     return AssertionSuccess();
   if(val->GetPointerSize() != 0)
     return AssertionFailure() << val->ToString() << " is allocated.";

   return AssertionSuccess();
 }

 template<typename T>
 static inline AssertionResult
 IsUnallocated(const Local<T>& val){
   return IsUnallocated(val.raw());
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

 static inline AssertionResult
 IsForwardingTo(RawObject* val, uword address){
   if(!val->IsForwarding())
     return AssertionFailure() << "Expected " << val->ToString() << " to be forwarding.";
   if(val->GetForwardingAddress() != address)
     return AssertionFailure() << "Expected " << val->ToString() << " to be forwarding to: " << ((void*)address);
   return AssertionSuccess();
 }

 static inline AssertionResult
 IsForwardingTo(RawObject* val, RawObject* dst){
   return IsForwardingTo(val, dst->GetAddress());
 }

 template<typename T>
 static inline AssertionResult
 IsForwarding(const Local<T>& val){
   return IsForwarding(val.raw());
 }

 static inline AssertionResult
 IsWord(RawObject* ptr, word value){
   if(ptr->GetPointerSize() != kWordSize)
     return AssertionFailure() << ptr->ToString() << " is not a word.";
   auto lhs = *((word*)ptr->GetPointer());
   if(lhs != value)
     return AssertionFailure() << ptr->ToString() << " (" << lhs << ") does not equal: " << value;
   return AssertionSuccess();
 }

 template<class T>
 static inline AssertionResult
 IsWord(const Local<T>& val, word value){
   return IsWord(val.raw(), value);
 }
}

#endif//POSEIDON_ASSERTIONS_H