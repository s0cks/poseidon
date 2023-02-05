#ifndef PSDN_PTR_ASSERTIONS_H
#define PSDN_PTR_ASSERTIONS_H

#include <gtest/gtest.h>
#include "poseidon/type.h"
#include "poseidon/local/local.h"

namespace poseidon {
#define FOR_EACH_POINTER_FLAG(V) \
 V(Marked)                       \
 V(Remembered)                   \
 V(Free)                         \
 V(New)                          \
 V(Old)

 static inline ::testing::AssertionResult
 IsAllocated(Pointer* ptr) {
   return (ptr == UNALLOCATED) ?
      ::testing::AssertionFailure() << "expected ptr to not be UNALLOCATED" :
      ::testing::AssertionSuccess() << "ptr is not UNALLOCATED";
 }

 template<class T>
 static inline ::testing::AssertionResult
 IsAllocated(T* value) {
   if(value == UNALLOCATED)
     return ::testing::AssertionFailure() << "expected value to not be UNALLOCATED";
   if(value->raw_ptr() == UNALLOCATED)
     return ::testing::AssertionFailure() << "expected " << (*value) << " to not be UNALLOCATED";
   return ::testing::AssertionSuccess();
 }

 static inline ::testing::AssertionResult
 IsUnallocated(Pointer* val){
   if(val == UNALLOCATED)
     return ::testing::AssertionSuccess();
   if(val->GetPointerSize() != 0)
     return ::testing::AssertionFailure() << (*val) << " is allocated.";
   return ::testing::AssertionSuccess();
 }

 static inline ::testing::AssertionResult
 IsNew(Pointer* val){
   if(!val->IsNew())
     return ::testing::AssertionFailure() << "Expected " << (*val) << " to be new.";
   return ::testing::AssertionSuccess();
 }

 template<typename T>
 static inline ::testing::AssertionResult
 IsNew(const Local<T>& local){
   return IsNew(local.raw());
 }

 static inline ::testing::AssertionResult
 IsOld(Pointer* val){
   if(!val->IsOld())
     return ::testing::AssertionFailure() << "Expected " << (*val) << " to be old.";
   return ::testing::AssertionSuccess();
 }

 template<typename T>
 static inline ::testing::AssertionResult
 IsOld(const Local<T>& val){
   return IsOld(val.raw());
 }

 static inline ::testing::AssertionResult
 IsMarked(Pointer* val){
   if(!val->IsMarked())
     return ::testing::AssertionFailure() << "Expected " << (*val) << " to marked.";
   return ::testing::AssertionSuccess();
 }

 template<typename T>
 static inline ::testing::AssertionResult
 IsMarked(const Local<T>& val){
   return IsMarked(val.raw_ptr());
 }

 static inline ::testing::AssertionResult
 IsRemembered(Pointer* val){
   if(!val->IsRemembered())
     return ::testing::AssertionFailure() << "Expected " << (*val) << " to be remembered.";
   return ::testing::AssertionSuccess();
 }

 template<typename T>
 static inline ::testing::AssertionResult
 IsRemembered(const Local<T>& val){
   return IsRemembered(val.raw_ptr());
 }

 static inline ::testing::AssertionResult
 IsForwarding(Pointer* val){
   if(!val->IsForwarding())
     return ::testing::AssertionFailure() << "Expected " << (*val) << " to be forwarding.";
   return ::testing::AssertionSuccess();
 }

 static inline ::testing::AssertionResult
 IsForwardingTo(Pointer* val, uword address){
   if(!val->IsForwarding())
     return ::testing::AssertionFailure() << "Expected " << (*val) << " to be forwarding.";
   if(val->GetForwardingAddress() != address)
     return ::testing::AssertionFailure() << "Expected " << (*val) << " to be forwarding to: " << ((void*)address);
   return ::testing::AssertionSuccess();
 }

 static inline ::testing::AssertionResult
 IsForwardingTo(Pointer* val, Pointer* dst){
   return IsForwardingTo(val, dst->GetStartingAddress());
 }

 template<typename T>
 static inline ::testing::AssertionResult
 IsForwarding(const Local<T>& val){
   return IsForwarding(val.raw_ptr());
 }

 static inline ::testing::AssertionResult
 IsWord(Pointer* ptr, word value){
   if(ptr->GetPointerSize() != kWordSize)
     return ::testing::AssertionFailure() << (*ptr) << " is not a word.";
   auto lhs = *((word*)ptr->GetPointer());
   if(lhs != value)
     return ::testing::AssertionFailure() << (*ptr) << " (" << lhs << ") does not equal: " << value;
   return ::testing::AssertionSuccess();
 }

 template<class T>
 static inline ::testing::AssertionResult
 IsWord(const Local<T>& val, word value){
   return IsWord(val.raw_ptr(), value);
 }

 static inline ::testing::AssertionResult
 IsNewWord(Pointer* ptr, word value){
   if(!IsAllocated(ptr))
     return IsAllocated(ptr);
   if(!ptr->IsNew())
     return ::testing::AssertionFailure() << (*ptr) << " is not new.";
   return IsWord(ptr, value);
 }

 template<class T>
 static inline ::testing::AssertionResult
 IsNewWord(const Local<T>& ptr, word value){
   return IsNewWord(ptr.raw_ptr(), value);
 }

 static inline ::testing::AssertionResult
 IsFree(Pointer* ptr) {
   if(!ptr->IsFree())
     return ::testing::AssertionFailure() << (*ptr) << " is not free";
   return ::testing::AssertionSuccess();
 }

 static inline ::testing::AssertionResult
 IsOldWord(Pointer* ptr, word value){
   if(!IsAllocated(ptr))
     return IsAllocated(ptr);
   if(!ptr->IsOld())
     return ::testing::AssertionFailure() << (*ptr) << " is not old.";
   return IsWord(ptr, value);
 }

 template<class T>
 static inline ::testing::AssertionResult
 IsOldWord(const Local<T>& ptr, const word value) {
   return IsOldWord(ptr.raw_ptr(), value);
 }

 static inline ::testing::AssertionResult
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
 static inline ::testing::AssertionResult
 IsMarkedWord(const Local<T>& ptr, word value){
   return IsMarkedWord(ptr.raw_ptr(), value);
 }

 static inline ::testing::AssertionResult
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
 static inline ::testing::AssertionResult
 IsRememberedWord(const Local<T>& ptr, word value){
   return IsRememberedWord(ptr.raw_ptr(), value);
 }
}

#endif //PSDN_PTR_ASSERTIONS_H