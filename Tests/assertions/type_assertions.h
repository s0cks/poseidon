#ifndef POSEIDON_INT_ASSERTIONS_H
#define POSEIDON_INT_ASSERTIONS_H

#include <gtest/gtest.h>
#include "poseidon/type.h"
#include "poseidon/local/local.h"

namespace poseidon {
 template<class T>
 static inline ::testing::AssertionResult
 IsUnallocated(const Local<T>& value){
   return value.IsEmpty() ?
          ::testing::AssertionSuccess() << (*value) << " is UNALLOCATED" :
          ::testing::AssertionFailure() << "expected " << (*value) << " to be UNALLOCATED";
 }

 template<class T>
 static inline ::testing::AssertionResult
 IsAllocated(const Local<T>& value){
   return value.IsEmpty() ?
          ::testing::AssertionFailure() << "expected " << *(value->raw_ptr()) << " to not be UNALLOCATED" :
          ::testing::AssertionSuccess() << *(value->raw_ptr()) << " is not UNALLOCATED";
 }

 template<class T>
 static inline ::testing::AssertionResult
 IsPtr(Pointer* ptr) {
   if(ptr == nullptr)
     return ::testing::AssertionFailure() << "expected " << T::GetClassName() << ", but was nullptr.";
   if(ptr->GetSize() != T::GetClassAllocationSize())
     return ::testing::AssertionFailure() << "expected an instanceof " << T::GetClass() << ", but was: " << (*ptr);
   return ::testing::AssertionSuccess();
 }

#define DEFINE_IS_TYPE_ASSERTION(Name) \
 static inline ::testing::AssertionResult Is##Name(Pointer* ptr) { return IsPtr<Name>(ptr); } \
 static inline ::testing::AssertionResult Is##Name(const Local<Name>& local) { return Is##Name(local->raw_ptr()); } \
 static inline ::testing::AssertionResult Is##Name(Name* ptr) { return Is##Name(ptr->raw_ptr()); }

 DEFINE_IS_TYPE_ASSERTION(Bool);
 FOR_EACH_INT_TYPE(DEFINE_IS_TYPE_ASSERTION);
 DEFINE_IS_TYPE_ASSERTION(Null);
 DEFINE_IS_TYPE_ASSERTION(Tuple);

 template<class T>
 static inline ::testing::AssertionResult
 PtrEq(T* expected, T* actual) {
   if(expected == nullptr && actual == nullptr)
     return ::testing::AssertionSuccess();
   if(expected == nullptr && actual != nullptr)
     return ::testing::AssertionFailure() << "expected nullptr, but was " << (*actual);
   if(expected != nullptr && actual == nullptr)
     return ::testing::AssertionFailure() << "expected " << (*expected) << ", but was nullptr";
   if(expected->Get() != actual->Get())
     return ::testing::AssertionFailure() << "expected " << (*expected) << ", but was " << (*actual);
   return ::testing::AssertionSuccess();
 }

#define DEFINE_TYPE_EQ_ASSERTION(Name) \
 static inline ::testing::AssertionResult Name##Eq(Name* expected, Name* actual) { return PtrEq<Name>(expected, actual); }

 DEFINE_TYPE_EQ_ASSERTION(Bool);
 DEFINE_TYPE_EQ_ASSERTION(Null);
 FOR_EACH_INT_TYPE(DEFINE_TYPE_EQ_ASSERTION);

#define DEFINE_TYPE_EQ_RAW_ASSERTION(Name) \
 static inline ::testing::AssertionResult  \
 Name##Eq(const Raw##Name expected, const Name* actual) { \
   if(expected == UNALLOCATED && actual == UNALLOCATED) \
     return ::testing::AssertionSuccess(); \
   if(actual == UNALLOCATED)               \
     return ::testing::AssertionFailure() << "expected " << expected << " but was UNALLOCATED"; \
   if(expected != actual->Get())           \
     return ::testing::AssertionFailure() << "expected " << expected << " but was " << (*actual); \
   return ::testing::AssertionSuccess();   \
 }                                         \
 static inline ::testing::AssertionResult  \
 Name##Eq(const Raw##Name expected, const Local<Name>& actual) {                                \
   return Name##Eq(expected, actual.Get());\
 }

 DEFINE_TYPE_EQ_RAW_ASSERTION(Bool);
 FOR_EACH_INT_TYPE(DEFINE_TYPE_EQ_RAW_ASSERTION);
}

#endif // POSEIDON_INT_ASSERTIONS_H