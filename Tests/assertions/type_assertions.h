#ifndef POSEIDON_INT_ASSERTIONS_H
#define POSEIDON_INT_ASSERTIONS_H

#ifndef POSEIDON_ASSERTIONS_H
#error "Please #include assertions/assertions.h instead"
#endif // POSEIDON_ASSERTIONS_H

#include <gtest/gtest.h>
#include "poseidon/type.h"

namespace poseidon {
#define DEFINE_IS_TYPE_ASSERTION(Name) \
 static inline ::testing::AssertionResult \
 Is##Name(Pointer* ptr) {              \
  if(ptr == UNALLOCATED)               \
    return ::testing::AssertionFailure() << "expected " << Name::kClassName << ", but was UNALLOCATED"; \
  if(ptr->GetSize() != Name::GetClass()->GetAllocationSize())                                           \
    return ::testing::AssertionFailure() << "sizeof " << (*ptr) << " does not equal " << Name::kClassName << "::GetClass()->GetAllocationSize() (" << Bytes(Name::GetClass()->GetAllocationSize()) << ")"; \
  return ::testing::AssertionSuccess();\
 }
 DEFINE_IS_TYPE_ASSERTION(Bool);
 DEFINE_IS_TYPE_ASSERTION(Byte);
 DEFINE_IS_TYPE_ASSERTION(Short);
 DEFINE_IS_TYPE_ASSERTION(Int);
 DEFINE_IS_TYPE_ASSERTION(Long);
 DEFINE_IS_TYPE_ASSERTION(Null);
 DEFINE_IS_TYPE_ASSERTION(Tuple);

#define DEFINE_TYPE_EQ_ASSERTION(Name) \
 static inline ::testing::AssertionResult \
 Name##Eq(Name* expected, Name* actual) { \
   if(expected == UNALLOCATED && actual == UNALLOCATED) \
     return ::testing::AssertionSuccess();\
   if(expected == UNALLOCATED && actual != UNALLOCATED) \
     return ::testing::AssertionFailure() << "expected UNALLOCATED but was: " << (*actual); \
   if(expected != UNALLOCATED && actual == UNALLOCATED) \
     return ::testing::AssertionFailure() << "expected " << (*expected) << " but was UNALLOCATED"; \
   if(expected->Get() != actual->Get())\
     return ::testing::AssertionFailure() << "expected " << (*expected) << " but was " << (*actual); \
   return ::testing::AssertionSuccess();  \
 }

 DEFINE_TYPE_EQ_ASSERTION(Bool);
 DEFINE_TYPE_EQ_ASSERTION(Null);

 DEFINE_TYPE_EQ_ASSERTION(Byte);
 DEFINE_TYPE_EQ_ASSERTION(Short);
 DEFINE_TYPE_EQ_ASSERTION(Int);
 DEFINE_TYPE_EQ_ASSERTION(Long);

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

 DEFINE_TYPE_EQ_RAW_ASSERTION(Byte);
 DEFINE_TYPE_EQ_RAW_ASSERTION(Short);
 DEFINE_TYPE_EQ_RAW_ASSERTION(Int);
 DEFINE_TYPE_EQ_RAW_ASSERTION(Long);
}

#endif // POSEIDON_INT_ASSERTIONS_H