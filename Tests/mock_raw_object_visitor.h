#ifndef POSEIDON_MOCK_RAW_OBJECT_VISITOR_H
#define POSEIDON_MOCK_RAW_OBJECT_VISITOR_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "poseidon/pointer.h"

namespace poseidon {
 class MockRawObjectVisitor : public RawObjectVisitor{
  private:
   static inline bool
   VisitPointer(Pointer* raw_ptr) {
     DLOG(INFO) << "visiting " << (*raw_ptr);
     return true;
   }
  public:
   MockRawObjectVisitor():
     RawObjectVisitor() {
     ON_CALL(*this, Visit(::testing::_))
       .WillByDefault(VisitPointer);
   }
   ~MockRawObjectVisitor() override = default;

   MOCK_METHOD(bool, Visit, (Pointer*), (override));
 };
}

#endif // POSEIDON_MOCK_RAW_OBJECT_VISITOR_H