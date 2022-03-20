#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <glog/logging.h>

#include "helpers.h"
#include "poseidon/allocator.h"

namespace poseidon{
 using namespace ::testing;

 class AllocatorTest : public Test{
  public:
   AllocatorTest() = default;
   ~AllocatorTest() override = default;
 };

 TEST_F(AllocatorTest, TestAllocateNewZone){
   auto val = Allocator::New<int>();
   (*val) = 100;

   ASSERT_EQ((*val), 100);
 }

 class RootPageTest : public Test{
  public:
   RootPageTest() = default;
   ~RootPageTest() override = default;
 };

 TEST_F(RootPageTest, TestContains){
   auto page = new RootPage();
   auto root = page->CreateReference();
   page->Contains((uword)root);
 }

 TEST_F(RootPageTest, TestVisitPointers){
   auto page = new RootPage();
   auto r1 = page->CreateReference();
   (*r1) = Allocator::Allocate(sizeof(uword));

   auto r2 = page->CreateReference();
   (*r2) = Allocator::Allocate(sizeof(uword));

   auto r3 = page->CreateReference();
   (*r3) = Allocator::Allocate(sizeof(uword));
 }
}