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
     return AssertionFailure() << (*val) << " is allocated.";

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
     return AssertionFailure() << "Expected " << (*val) << " to be new.";
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
     return AssertionFailure() << "Expected " << (*val) << " to be old.";
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
     return AssertionFailure() << "Expected " << (*val) << " to marked.";
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
     return AssertionFailure() << "Expected " << (*val) << " to be remembered.";
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
     return AssertionFailure() << "Expected " << (*val) << " to be forwarding.";
   return AssertionSuccess();
 }

 static inline AssertionResult
 IsForwardingTo(RawObject* val, uword address){
   if(!val->IsForwarding())
     return AssertionFailure() << "Expected " << (*val) << " to be forwarding.";
   if(val->GetForwardingAddress() != address)
     return AssertionFailure() << "Expected " << (*val) << " to be forwarding to: " << ((void*)address);
   return AssertionSuccess();
 }

 static inline AssertionResult
 IsForwardingTo(RawObject* val, RawObject* dst){
   return IsForwardingTo(val, dst->GetStartingAddress());
 }

 template<typename T>
 static inline AssertionResult
 IsForwarding(const Local<T>& val){
   return IsForwarding(val.raw());
 }

 static inline AssertionResult
 IsWord(RawObject* ptr, word value){
   if(ptr->GetPointerSize() != kWordSize)
     return AssertionFailure() << (*ptr) << " is not a word.";
   auto lhs = *((word*)ptr->GetPointer());
   if(lhs != value)
     return AssertionFailure() << (*ptr) << " (" << lhs << ") does not equal: " << value;
   return AssertionSuccess();
 }

 template<class T>
 static inline AssertionResult
 IsWord(const Local<T>& val, word value){
   return IsWord(val.raw(), value);
 }

 static inline AssertionResult
 IsNewWord(RawObject* ptr, word value){
   if(!IsAllocated(ptr))
     return IsAllocated(ptr);
   if(!ptr->IsNew())
     return AssertionFailure() << (*ptr) << " is not new.";
   return IsWord(ptr, value);
 }

 template<class T>
 static inline AssertionResult
 IsNewWord(const Local<T>& ptr, word value){
   return IsNewWord(ptr.raw(), value);
 }

 static inline AssertionResult
 IsOldWord(RawObject* ptr, word value){
   if(!IsAllocated(ptr))
     return IsAllocated(ptr);
   if(!ptr->IsOld())
     return AssertionFailure() << (*ptr) << " is not old.";
   return IsWord(ptr, value);
 }

 static inline AssertionResult
 IsMarkedWord(RawObject* ptr, word value){
   if(!IsAllocated(ptr))
     return IsAllocated(ptr);
   if(!IsOld(ptr))
     return IsOld(ptr);
   if(!IsMarked(ptr))
     return IsMarked(ptr);
   return IsWord(ptr, value);
 }

 template<class T>
 static inline AssertionResult
 IsMarkedWord(const Local<T>& ptr, word value){
   return IsMarkedWord(ptr.raw(), value);
 }

 static inline AssertionResult
 IsRememberedWord(RawObject* ptr, word value){
   if(!IsAllocated(ptr))
     return IsAllocated(ptr);
   if(!IsNew(ptr))
     return IsNew(ptr);
   if(!IsRemembered(ptr))
     return IsRemembered(ptr);
   return IsWord(ptr, value);
 }

 template<class T>
 static inline AssertionResult
 IsRememberedWord(const Local<T>& ptr, word value){
   return IsRememberedWord(ptr.raw(), value);
 }
}

#endif//POSEIDON_ASSERTIONS_H