#ifndef POSEIDON_TEST_PAGE_H
#define POSEIDON_TEST_PAGE_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "poseidon/heap/page.h"

namespace poseidon {
 using namespace ::testing;

 class PageTest : public Test {
  protected:
   PageTest() = default;

   static inline void
   Mark(Page* page) {
     page->SetMarkedBit(true);
   }

   static inline void
   Unmark(Page* page) {
     page->ClearMarkedBit();
   }

   static inline void
   SetIndex(Page* page, const uint32_t index) {
     page->SetIndex(index);
   }
  public:
   ~PageTest() override = default;
 };
}

#endif // POSEIDON_TEST_PAGE_H