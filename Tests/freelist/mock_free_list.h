#ifndef POSEIDON_MOCK_FREE_LIST_H
#define POSEIDON_MOCK_FREE_LIST_H

#include <gmock/gmock.h>
#include "poseidon/freelist/freelist.h"

namespace poseidon {
 class MockFreeList : public FreeList {
  public:
   MockFreeList() = default;
   ~MockFreeList() override = default;
 };
}

#endif // POSEIDON_MOCK_FREE_LIST_H