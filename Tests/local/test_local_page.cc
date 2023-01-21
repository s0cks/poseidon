#include <gtest/gtest.h>
#include "poseidon/pointer.h"
#include "poseidon/local/local_page.h"

#include "poseidon/heap/heap.h"
#include "poseidon/type/int.h"

namespace poseidon {
 using namespace ::testing;

 class LocalPageTest : public Test {
  protected:
   LocalPageTest() = default;
  public:
   ~LocalPageTest() override = default;
 };

 TEST_F(LocalPageTest, TestConstructor) {
   MemoryRegion region(LocalPage::CalculateLocalPageSize(128));
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   LocalPage page(region);

   auto heap = Heap::GetCurrentThreadHeap();
   auto new_zone = heap->new_zone();

   static constexpr const int64_t kALocalIndex = 0;
   static constexpr const RawInt kAValue = 44;
   auto a = Int::New(kAValue);
   DLOG(INFO) << "a: " << (*a);
   ASSERT_TRUE(new_zone->Intersects((Region) *a->raw_ptr()));
   ASSERT_EQ(a->Get(), kAValue);
   ASSERT_NO_FATAL_FAILURE(page.SetLocal(kALocalIndex, a->raw_ptr()));

   static constexpr const int64_t kBLocalIndex = 1;
   static constexpr const RawInt kBValue = 88;
   auto b = Int::New(kBValue);
   DLOG(INFO) << "b: " << (*b);
   ASSERT_TRUE(new_zone->Intersects((Region) *b->raw_ptr()));
   ASSERT_EQ(b->Get(), kBValue);
   ASSERT_NO_FATAL_FAILURE(page.SetLocal(kBLocalIndex, b->raw_ptr()));

   auto c_ptr = page.GetLocal(kALocalIndex);
   auto c = (Int*)c_ptr->GetPointer();
   ASSERT_EQ(c->Get(), kAValue);

   auto d_ptr = page.GetLocal(kBLocalIndex);
   auto d = (Int*)d_ptr->GetPointer();
   ASSERT_EQ(d->Get(), kBValue);
 }
}