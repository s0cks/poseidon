#ifndef PSDN_IS_CLASS_H
#define PSDN_IS_CLASS_H

#include <gtest/gtest.h>
#include "poseidon/object.h"

namespace poseidon {
 class IsClassMatcher {
  protected:
   Class* cls_;
  public:
   using is_gtest_matcher = void;

   IsClassMatcher(Class* cls):
    cls_(cls) {
   }
   virtual ~IsClassMatcher() = default;

   Class* GetClass() const {
     return cls_;
   }

   bool MatchAndExplain(Class* cls, std::ostream*) const {
     DLOG(INFO) << "checking " << (*cls) << " against " << (*GetClass());
     if(cls->GetName() != GetClass()->GetName()) {
       DLOG(WARNING) << "GetName() is not equal";
       return false;
     }
     if(cls->GetAllocationSize() != GetClass()->GetAllocationSize()) {
       DLOG(WARNING) << "GetAllocationSize() is not equal";
       return false;
     }
     DLOG(INFO) << "found match";
     return true;
   }

   void DescribeTo(std::ostream* stream) const {
     (*stream) <<  "equals " << (*GetClass());
   }

   void DescribeNegationTo(std::ostream* stream) const {
     (*stream) << "does not equal " << (*GetClass());
   }
 };

 template<class T>
 static inline ::testing::Matcher<Class*>
 IsClass() {
   return IsClassMatcher(T::GetClass());
 }
}

#endif //PSDN_IS_CLASS_H