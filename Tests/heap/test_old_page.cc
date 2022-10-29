//#include "helpers.h"
//#include "helpers/assertions.h"
//
//namespace poseidon{
//#define UNALLOCATED 0 //TODO: cleanup
//
// using namespace ::testing;
//
// class OldPageTest : public Test {
//  protected:
//   OldPageTest() = default;
//
//   static inline uword
//   TryAllocateBytes(OldPage* page, const ObjectSize size) {
//     return page->TryAllocate(size);
//   }
//
//   static inline RawObject*
//   TryAllocateWord(OldPage* page, word value) {
//     auto address = TryAllocateBytes(page, kWordSize);
//     if (address == UNALLOCATED)
//       return nullptr;
//     auto ptr = (RawObject*)address;
//     (*((word*)ptr->GetObjectPointerAddress())) = value;
//     return ptr;
//   }
//
//   static inline RawObject*
//   TryAllocateMarkedWord(OldPage* page, word value) {
//     auto address = TryAllocateBytes(page, kWordSize);
//     if (address == UNALLOCATED)
//       return nullptr;
//     auto ptr = (RawObject*)address;
//     ptr->SetMarkedBit();
//     (*((word*)ptr->GetObjectPointerAddress())) = value;
//     return ptr;
//   }
//
//   static inline void
//   SetTag(OldPage* page, const PageTag& value) {
//     page->SetTag(value.raw());
//   }
//  public:
//   ~OldPageTest() override = default;
// };
//
// TEST_F(OldPageTest, TestConstructor) {
//   static const constexpr PageIndex kPageIndex = 0;
//   MemoryRegion region(GetOldPageSize());
//   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
//   auto page = OldPage::From(kPageIndex, region);
//   NOT_IMPLEMENTED(ERROR); //TODO: implement
// }
//
// TEST_F(OldPageTest, TestTryAllocateWillFail_LessThanZero) {
//   static const constexpr PageIndex kPageIndex = 0;
//   MemoryRegion region(GetOldPageSize());
//   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
//   auto page = OldPage::From(kPageIndex, region);
//   auto ptr = TryAllocateBytes(page, -1);
//   ASSERT_EQ(ptr, UNALLOCATED);
// }
//
// TEST_F(OldPageTest, TestTryAllocateWillFail_EqualToZero) {
//   static const constexpr PageIndex kPageIndex = 0;
//   MemoryRegion region(GetOldPageSize());
//   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
//   auto page = OldPage::From(kPageIndex, region);
//   auto ptr = TryAllocateBytes(page, 0);
//   ASSERT_EQ(ptr, UNALLOCATED);
// }
//
// TEST_F(OldPageTest, TestTryAllocateWillFail_EqualToPageSize) {
//   static const constexpr PageIndex kPageIndex = 0;
//   MemoryRegion region(GetOldPageSize());
//   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
//   auto page = OldPage::From(kPageIndex, region);
//   auto ptr = TryAllocateBytes(page, GetOldPageSize());
//   ASSERT_EQ(ptr, UNALLOCATED);
// }
//
// TEST_F(OldPageTest, TestTryAllocateWillFail_GreaterThanPageSize) {
//   static const constexpr PageIndex kPageIndex = 0;
//   MemoryRegion region(GetOldPageSize());
//   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
//   auto page = OldPage::From(kPageIndex, region);
//   auto ptr = TryAllocateBytes(page, GetOldPageSize() + 1);
//   ASSERT_EQ(ptr, UNALLOCATED);
// }
//
// TEST_F(OldPageTest, TestTryAllocate){
//   static const constexpr PageIndex kPageIndex = 0;
//   MemoryRegion region(GetOldPageSize());
//   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
//   auto page = OldPage::From(kPageIndex, region);
//
//   static const constexpr word kDefaultWordValue = 42;
//   auto ptr = TryAllocateWord(page, kDefaultWordValue);
//   ASSERT_TRUE(IsAllocated(ptr));
//   ASSERT_FALSE(IsNew(ptr));
//   ASSERT_TRUE(IsOld(ptr));
//   ASSERT_FALSE(IsMarked(ptr));
//   ASSERT_FALSE(IsRemembered(ptr));
//   ASSERT_FALSE(IsForwarding(ptr));
//   ASSERT_EQ(ptr->GetPointerSize(), kWordSize);
// }
//
// TEST_F(OldPageTest, TestVisitPointers){
//   static const constexpr PageIndex kPageIndex = 0;
//   MemoryRegion region(GetOldPageSize());
//   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
//   auto page = OldPage::From(kPageIndex, region);
//
//   static const constexpr int64_t kNumberOfPointers = 3;
//   for(auto idx = 0; idx < kNumberOfPointers; idx++){
//     auto ptr = TryAllocateWord(page, idx);
//     ASSERT_TRUE(IsAllocated(ptr));
//   }
//
//   MockRawObjectVisitor visitor;
//   EXPECT_CALL(visitor, Visit)
//     .Times(kNumberOfPointers);
//   ASSERT_NO_FATAL_FAILURE(page->VisitPointers(&visitor));
// }
//
// TEST_F(OldPageTest, TestVisitMarkedPointers){
//   static const constexpr PageIndex kPageIndex = 0;
//   MemoryRegion region(GetOldPageSize());
//   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
//   auto page = OldPage::From(kPageIndex, region);
//
//   static const constexpr int64_t kNumberOfUnmarkedPointers = 1;
//   static const constexpr int64_t kNumberOfMarkedPointers = 3;
//
//   for(auto idx = 0; idx < kNumberOfUnmarkedPointers; idx++){
//     auto ptr = TryAllocateWord(page, idx);
//     ASSERT_TRUE(IsAllocated(ptr));
//     ASSERT_TRUE(page->Contains(ptr->GetStartingAddress()));
//     ASSERT_TRUE(IsOld(ptr));
//     ASSERT_TRUE(IsWord(ptr, idx));
//     ASSERT_FALSE(IsMarked(ptr));
//   }
//
//   for(auto idx = 0; idx < kNumberOfMarkedPointers; idx++){
//     auto ptr = TryAllocateMarkedWord(page, idx);
//     ASSERT_TRUE(IsAllocated(ptr));
//     ASSERT_TRUE(page->Contains(ptr->GetStartingAddress()));
//     ASSERT_TRUE(IsOld(ptr));
//     ASSERT_TRUE(IsWord(ptr, idx));
//     ASSERT_TRUE(IsMarked(ptr));
//   }
//
//   MockRawObjectVisitor visitor;
//   EXPECT_CALL(visitor, Visit)
//     .Times(kNumberOfMarkedPointers);
//   ASSERT_NO_FATAL_FAILURE(page->VisitMarkedPointers(&visitor));
// }
//}