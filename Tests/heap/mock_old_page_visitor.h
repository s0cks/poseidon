#ifndef POSEIDON_MOCK_OLD_PAGE_VISITOR_H
#define POSEIDON_MOCK_OLD_PAGE_VISITOR_H

#include <gmock/gmock.h>
#include "poseidon/page/old_page.h"

namespace poseidon {
 class MockOldPageVisitor : public OldPageVisitor {
  public:
   MockOldPageVisitor():
    OldPageVisitor() {
     ON_CALL(*this, Visit)
      .WillByDefault(::testing::Return(true));
   }
   ~MockOldPageVisitor() override = default;
   MOCK_METHOD(bool, Visit, (OldPage*), (override));
 };
}

#endif // POSEIDON_MOCK_PAGE_VISITOR_H