#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "poseidon/heap/freelist_node.h"

namespace poseidon {
 using namespace ::testing;

 class FreeListNodeTest : public Test {
  protected:
   FreeListNodeTest() = default;
  public:
   ~FreeListNodeTest() override = default;
 };

 TEST_F(FreeListNodeTest, TestConstructor) {
   MemoryRegion region(1 * kMB);
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));

   auto node = FreeListNode::NewNode(region);
   ASSERT_EQ(node->GetStartingAddress(), region.GetStartingAddress());
   ASSERT_EQ(node->GetSize(), region.GetSize());
   ASSERT_EQ(node->GetEndingAddress(), region.GetEndingAddress());
   ASSERT_EQ(node->GetNextAddress(), node->GetEndingAddress());
 }

 TEST_F(FreeListNodeTest, TestEquals) {
   MemoryRegion region(1 * kMB);
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));

   auto a = FreeListNode::NewNode(region);
   auto b = FreeListNode::NewNode(region);
   ASSERT_EQ(a, b);
 }
}