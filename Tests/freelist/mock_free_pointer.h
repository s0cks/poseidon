#ifndef POSEIDON_MOCK_FREE_POINTER_H
#define POSEIDON_MOCK_FREE_POINTER_H

#include <gmock/gmock.h>
#include "poseidon/freelist/free_ptr.h"

namespace poseidon {
 class MockFreePointer : public FreePointer {
  public:
   MockFreePointer() = default;
   ~MockFreePointer() override = default;
 };

 class MockFreePointerVisitor : public FreePointerVisitor {
  public:
   explicit MockFreePointerVisitor() = default;
   ~MockFreePointerVisitor() override = default;
   MOCK_METHOD(bool, VisitFreePointer, (FreePointer*), ());
 };
}

#endif // POSEIDON_MOCK_FREE_POINTER_H