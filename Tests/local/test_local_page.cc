#include <gtest/gtest.h>
#include "poseidon/local/local_page.h"

#include "poseidon/heap/heap.h"
#include "poseidon/type.h"

namespace poseidon {
 using namespace ::testing;

 class LocalPageTest : public Test {
  protected:
   LocalPage* page_;

   inline LocalPage* page() const {
     return page_;
   }
  public:
   LocalPageTest():
    Test(),
    page_(nullptr) {
   }
  ~LocalPageTest() override = default;

   void SetUp() override {
     ASSERT_NE(page_ = LocalPage::New(), nullptr);
     ASSERT_TRUE(page_->IsEmpty());
   }

   void TearDown() override {
     ASSERT_NO_FATAL_FAILURE(delete page_);
   }
 };

 //TODO: implement some tests
}