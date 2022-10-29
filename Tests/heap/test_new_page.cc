#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "poseidon/heap/new_page.h"

#include "helpers.h"

namespace poseidon {
#define UNALLOCATED 0 //TODO: cleanup

 using namespace ::testing;

 class NewPageTest : public Test {
  protected:
   NewPageTest() = default;
  public:
   ~NewPageTest() override = default;
 };

 TEST_F(NewPageTest, TestConstructor) {
   static const constexpr int64_t kPageIndex = 0;
   MemoryRegion region(GetNewPageSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto page = NewPage::New(region);
   ASSERT_EQ(page->GetStartingAddress(), region.GetStartingAddress());
   ASSERT_EQ(page->GetSize(), region.GetSize());
 }
}