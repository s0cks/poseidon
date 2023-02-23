#ifndef PSDN_PTR_ASSERTIONS_H
#define PSDN_PTR_ASSERTIONS_H

#include <gtest/gtest.h>
#include "poseidon/object.h"
#include "poseidon/local/local.h"

namespace poseidon{
 static inline ::testing::AssertionResult
 IsAllocated(uword address){
   if(address == UNALLOCATED)
     return ::testing::AssertionFailure() << "expected " << address << " (" << ((void*) address) << ") to not be UNALLOCATED";
   return ::testing::AssertionSuccess();
 }

 static inline ::testing::AssertionResult
 IsAllocated(Pointer * ptr){
   if(ptr == UNALLOCATED)
     return ::testing::AssertionFailure() << "expected ptr to not be UNALLOCATED";
   return ::testing::AssertionSuccess();
 }

 template<class T>
 static inline ::testing::AssertionResult
 IsAllocated(Local<T>& local) {
   if(local.IsEmpty())
     return ::testing::AssertionFailure() << "expected " << local << " to not be UNALLOCATED";
   return IsAllocated(local.raw_ptr());
 }

 template<class T>
 static inline ::testing::AssertionResult
 IsAllocated(T* value){
   if(value == UNALLOCATED)
     return ::testing::AssertionFailure() << "expected value to not be UNALLOCATED";
   if(value->raw_ptr() == UNALLOCATED)
     return ::testing::AssertionFailure() << "expected " << (*value) << " to not be UNALLOCATED";
   return ::testing::AssertionSuccess();
 }

 static inline ::testing::AssertionResult
 IsUnallocated(uword address){
   if(address != UNALLOCATED)
     return ::testing::AssertionFailure() << address << " " << ((void*) address) << " is allocated";
   return ::testing::AssertionSuccess();
 }

 static inline ::testing::AssertionResult
 IsUnallocated(Pointer * val){
   if(val == UNALLOCATED)
     return ::testing::AssertionSuccess();
   if(val->GetPointerSize() != 0)
     return ::testing::AssertionFailure() << (*val) << " is allocated.";
   return ::testing::AssertionSuccess();
 }

 template<class T>
 static inline ::testing::AssertionResult
 IsUnallocated(Local<T>& value) {
   return IsUnallocated(value.raw_ptr());
 }

 template<class T>
 static inline ::testing::AssertionResult
 IsUnallocated(T* value) {
   if(value == UNALLOCATED)
     return ::testing::AssertionSuccess();
   return IsUnallocated(value->raw_ptr());
 }

#define DEFINE_POINTER_TAG_BIT_CHECK(Name) \
 static inline ::testing::AssertionResult  \
 Is##Name(Pointer* ptr) {                  \
   if(ptr->Is##Name())                     \
    return ::testing::AssertionSuccess();  \
   return ::testing::AssertionFailure() << "expected " << (*ptr) << " to be " << #Name; \
 }

#define DEFINE_LOCAL_POINTER_TAG_BIT_CHECK(Name) \
 template<class T>                               \
 static inline ::testing::AssertionResult        \
 Is##Name(Local<T>& local) {                     \
  return Is##Name(local.raw_ptr());              \
 }

#define DEFINE_TYP_POINTER_TAG_BIT_CHECK(Name) \
 template<class T>                             \
 static inline ::testing::AssertionResult      \
 Is##Name(T* value) {                          \
  return Is##Name(value->raw_ptr());           \
 }

 FOR_EACH_POINTER_TAG_BIT(DEFINE_POINTER_TAG_BIT_CHECK);
 FOR_EACH_POINTER_TAG_BIT(DEFINE_LOCAL_POINTER_TAG_BIT_CHECK);
 FOR_EACH_POINTER_TAG_BIT(DEFINE_TYP_POINTER_TAG_BIT_CHECK);

 static inline ::testing::AssertionResult
 IsForwarding(Pointer* val){
   if(!val->IsForwarding())
     return ::testing::AssertionFailure() << "Expected " << (*val) << " to be forwarding.";
   return ::testing::AssertionSuccess();
 }

 template<class T>
 static inline ::testing::AssertionResult
 IsForwarding(Local<T>& local) {
   return IsForwarding(local.raw_ptr());
 }

 template<class T>
 static inline ::testing::AssertionResult
 IsForwarding(T* value) {
   return IsForwarding(value->raw_ptr());
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