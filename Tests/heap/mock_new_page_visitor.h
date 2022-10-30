#ifndef POSEIDON_MOCK_NEW_PAGE_VISITOR_H
#define POSEIDON_MOCK_NEW_PAGE_VISITOR_H

#include <gmock/gmock.h>
#include "poseidon/heap/new_page.h"

namespace poseidon {
 class MockNewPageVisitor : public NewPageVisitor {
  public:
   MockNewPageVisitor():
    NewPageVisitor() {
     ON_CALL(*this, Visit)
      .WillByDefault(::testing::Return(true));
   }
   ~MockNewPageVisitor() override = default;
   MOCK_METHOD(bool, Visit, (NewPage*), (override));
 };
}

#endif // POSEIDON_MOCK_NEW_PAGE_VISITOR_H