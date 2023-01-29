#ifndef POSEIDON_ASSERTIONS_H
#define POSEIDON_ASSERTIONS_H

#include <gtest/gtest.h>

#include "poseidon/type.h"
#include "poseidon/local/local.h"

namespace poseidon{ //TODO: cleanup & organize this file
 using namespace ::testing;

 static inline AssertionResult
 IsAllocated(Pointer* val){
   if(val == UNALLOCATED)
     return AssertionFailure() << "Expected to be allocated.";
   return AssertionSuccess();
 }

 template<class T>
 static inline AssertionResult
 IsAllocated(const Local<T>& val){
   return IsAllocated(val.raw_ptr());
 }

 static inline AssertionResult
 IsUnallocated(Pointer* val){
   if(val == UNALLOCATED)
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
 IsNew(Pointer* val){
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
 IsOld(Pointer* val){
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
 IsMarked(Pointer* val){
   if(!val->IsMarked())
     return AssertionFailure() << "Expected " << (*val) << " to marked.";
   return AssertionSuccess();
 }

 template<typename T>
 static inline AssertionResult
 IsMarked(const Local<T>& val){
   return IsMarked(val.raw_ptr());
 }

 static inline AssertionResult
 IsRemembered(Pointer* val){
   if(!val->IsRemembered())
     return AssertionFailure() << "Expected " << (*val) << " to be remembered.";
   return AssertionSuccess();
 }

 template<typename T>
 static inline AssertionResult
 IsRemembered(const Local<T>& val){
   return IsRemembered(val.raw_ptr());
 }

 static inline AssertionResult
 IsForwarding(Pointer* val){
   if(!val->IsForwarding())
     return AssertionFailure() << "Expected " << (*val) << " to be forwarding.";
   return AssertionSuccess();
 }

 static inline AssertionResult
 IsForwardingTo(Pointer* val, uword address){
   if(!val->IsForwarding())
     return AssertionFailure() << "Expected " << (*val) << " to be forwarding.";
   if(val->GetForwardingAddress() != address)
     return AssertionFailure() << "Expected " << (*val) << " to be forwarding to: " << ((void*)address);
   return AssertionSuccess();
 }

 static inline AssertionResult
 IsForwardingTo(Pointer* val, Pointer* dst){
   return IsForwardingTo(val, dst->GetStartingAddress());
 }

 template<typename T>
 static inline AssertionResult
 IsForwarding(const Local<T>& val){
   return IsForwarding(val.raw_ptr());
 }

 static inline AssertionResult
 IsWord(Pointer* ptr, word value){
   if(ptr->GetPointerSize() != kWordSize)
     return AssertionFailure() << (*ptr) << " is not a word.";
   auto lhs = *((word*)ptr->GetPointer());
   if(lhs != value)
     return AssertionFailure() << (*ptr) << " (" << lhs << ") does not equal: " << value;
   return AssertionSuccess();
 }

 static inline AssertionResult
 IsLong(Pointer* ptr, RawLong value) {
   if(ptr->GetPointerSize() != Class::kLongClass->GetAllocationSize())
     return AssertionFailure() << (*ptr) << " is not a word.";
   auto lhs = (Long*)ptr->GetPointer();
   if(lhs->Get() != value)
     return AssertionFailure() << (*ptr) << " (" << lhs->Get() << ") does not equal: " << value;
   return AssertionSuccess();
 }

 template<class T>
 static inline AssertionResult
 IsWord(const Local<T>& val, word value){
   return IsWord(val.raw_ptr(), value);
 }

 static inline AssertionResult
 IsNewWord(Pointer* ptr, word value){
   if(!IsAllocated(ptr))
     return IsAllocated(ptr);
   if(!ptr->IsNew())
     return AssertionFailure() << (*ptr) << " is not new.";
   return IsWord(ptr, value);
 }

 static inline AssertionResult
 IsNewLong(Pointer* ptr, RawLong value) {
   if(IsUnallocated(ptr))
     return AssertionFailure() << (*ptr) << " is unallocated";
   if(!ptr->IsNew())
     return AssertionFailure() << "expected " << (*ptr) << " to be new";
   return IsLong(ptr, value);
 }

 template<class T>
 static inline AssertionResult
 IsNewWord(const Local<T>& ptr, word value){
   return IsNewWord(ptr.raw_ptr(), value);
 }

 template<class T>
 static inline AssertionResult
 IsNewLong(const Local<T>& ptr, RawLong value) {
   return IsNewLong(ptr.raw_ptr(), value);
 }

 static inline AssertionResult
 IsFree(Pointer* ptr) {
   if(!ptr->IsFree())
     return AssertionFailure() << (*ptr) << " is not free";
   return AssertionSuccess();
 }

 static inline AssertionResult
 IsOldWord(Pointer* ptr, word value){
   if(!IsAllocated(ptr))
     return IsAllocated(ptr);
   if(!ptr->IsOld())
     return AssertionFailure() << (*ptr) << " is not old.";
   return IsWord(ptr, value);
 }

 template<class T>
 static inline AssertionResult
 IsOldWord(const Local<T>& ptr, const word value) {
   return IsOldWord(ptr.raw_ptr(), value);
 }

 static inline AssertionResult
 IsMarkedWord(Pointer* ptr, word value){
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
   return IsMarkedWord(ptr.raw_ptr(), value);
 }

 static inline AssertionResult
 IsRememberedWord(Pointer* ptr, word value){
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
   return IsRememberedWord(ptr.raw_ptr(), value);
 }
}

#endif//POSEIDON_ASSERTIONS_H