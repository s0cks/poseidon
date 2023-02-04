#ifndef POSEIDON_INT_ASSERTIONS_H
#define POSEIDON_INT_ASSERTIONS_H

#ifndef POSEIDON_ASSERTIONS_H
#error "Please #include assertions/assertions.h instead"
#endif // POSEIDON_ASSERTIONS_H

#include <gtest/gtest.h>
#include "poseidon/type.h"

namespace poseidon {
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
 static inline ::testing::AssertionResult Is##Name(Pointer* ptr) { return IsPtr<Name>(ptr); }

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
 }

 DEFINE_TYPE_EQ_RAW_ASSERTION(Bool);
 FOR_EACH_INT_TYPE(DEFINE_TYPE_EQ_RAW_ASSERTION);
}

#endif // POSEIDON_INT_ASSERTIONS_H