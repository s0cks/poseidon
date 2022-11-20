#ifndef POSEIDON_HELPERS_H
#define POSEIDON_HELPERS_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <glog/logging.h>

#include "poseidon/heap/heap.h"

#include "helpers/assertions.h"

namespace poseidon{
 using namespace ::testing;

 class MockFreeListNodeVisitor : public FreeObjectVisitor {
  public:
   MockFreeListNodeVisitor():
       FreeObjectVisitor() {
     ON_CALL(*this, Visit)
      .WillByDefault([](FreeObject* val) {
        return true;
      });
   }
   ~MockFreeListNodeVisitor() override = default;
   MOCK_METHOD(bool, Visit, (FreeObject*), (override));
 };
}

#endif //POSEIDON_HELPERS_H
