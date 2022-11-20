#ifndef POSEIDON_INT_ASSERTIONS_H
#define POSEIDON_INT_ASSERTIONS_H

#ifndef POSEIDON_ASSERTIONS_H
#error "Please #include assertions/assertions.h instead"
#endif // POSEIDON_ASSERTIONS_H

#include <gtest/gtest.h>
#include "poseidon/type/int.h"

namespace poseidon {
 static inline ::testing::AssertionResult
 IsInt(RawObject* ptr) {
   if(ptr == UNALLOCATED)
     return ::testing::AssertionFailure() << "not allocated";
   if(ptr->GetSize() != Int::GetClass()->GetAllocationSize())
     return ::testing::AssertionFailure() << "size of " << (*ptr) << " does not equal Int::GetClass()->GetAllocationSize() (" << Bytes(Int::GetClass()->GetAllocationSize()) << ")";
   return ::testing::AssertionSuccess();
 }

 static inline ::testing::AssertionResult
 IntEq(Int* expected, Int* actual) {
   if(expected == UNALLOCATED && actual == UNALLOCATED)
     return ::testing::AssertionSuccess();
   if(expected == UNALLOCATED && actual != UNALLOCATED)
     return ::testing::AssertionFailure() << "expected UNALLOCATED but was: " << (*actual);
   if(expected != UNALLOCATED && actual == UNALLOCATED)
     return ::testing::AssertionFailure() << "expected " << (*expected) << " but was UNALLOCATED";
   if(expected->Get() != actual->Get())
     return ::testing::AssertionFailure() << "expected " << (*expected) << " but was " << (*actual);
   return ::testing::AssertionSuccess();
 }
}

#endif // POSEIDON_INT_ASSERTIONS_H