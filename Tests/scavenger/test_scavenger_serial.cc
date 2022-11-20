#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "helpers.h"
#include "matchers/is_pointer_to.h"
#include "helpers/assertions.h"
#include "helpers/alloc_helpers.h"
#include "scavenger/mock_scavenger.h"
#include "poseidon/scavenger/scavenger_serial.h"

namespace poseidon {
 using namespace ::testing;

 class SerialScavengerTest : public Test {
  protected:
   SerialScavengerTest() = default;

   static inline bool
   SerialScavenge(Scavenger* scavenger) {
     SerialScavenger serial_scavenger(scavenger);
     return serial_scavenger.Scavenge();
   }

   static inline bool
   SerialScavenge(Heap* heap) {
     Scavenger scavenger(heap);
     return SerialScavenge(&scavenger);
   }
  public:
   ~SerialScavengerTest() override = default;
 };

 TEST_F(SerialScavengerTest, TestSerialScavenge_WillPass_DoesNothing) {
   MemoryRegion page_region(LocalPage::CalculateLocalPageSize(32));
   ASSERT_TRUE(page_region.Protect(MemoryRegion::kReadWrite));
   LocalPage page(page_region);
   ASSERT_NO_FATAL_FAILURE(LocalPage::SetForCurrentThread(&page));

   MemoryRegion region(GetTotalInitialHeapSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto heap = Heap::From(region);
   auto zone = heap->new_zone();
   auto fromspace = zone->fromspace();
   auto tospace = zone->tospace();

   static constexpr const word kAValue = 33;
   auto a = TryAllocateWord(zone, kAValue);
   ASSERT_TRUE(IsNewWord(a, kAValue));
   ASSERT_FALSE(IsMarked(a));

   MockScavenger scavenger(heap);
   ASSERT_TRUE(SerialScavenge(&scavenger));

   EXPECT_CALL(scavenger, Scavenge(_))
    .Times(0);

   // scavenging should always flip the semi-spaces
   ASSERT_EQ(fromspace.GetStartingAddress(), zone->tospace().GetStartingAddress());
   ASSERT_EQ(fromspace.GetSize(), zone->tospace().GetSize());

   ASSERT_EQ(tospace.GetStartingAddress(), zone->fromspace().GetStartingAddress());
   ASSERT_EQ(tospace.GetSize(), zone->fromspace().GetSize());

   ASSERT_NO_FATAL_FAILURE(LocalPage::SetForCurrentThread(nullptr));
 }

 TEST_F(SerialScavengerTest, TestSerialScavenge_WillPass_ScavengesOneObject) {
   MemoryRegion page_region(LocalPage::CalculateLocalPageSize(32));
   ASSERT_TRUE(page_region.Protect(MemoryRegion::kReadWrite));
   LocalPage page(page_region);
   ASSERT_NO_FATAL_FAILURE(LocalPage::SetForCurrentThread(&page));

   MemoryRegion region(GetTotalInitialHeapSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto heap = Heap::From(region);
   auto zone = heap->new_zone();
   auto fromspace = zone->fromspace();
   auto tospace = zone->tospace();

   static constexpr const word kAValue = 33;
   Local<word> a(TryAllocateMarkedWord(zone, kAValue));
   DLOG(INFO) << "a: " << (*a.raw_ptr());
   ASSERT_TRUE(IsNewWord(a, kAValue));
   ASSERT_TRUE(IsMarked(a));
   ASSERT_FALSE(IsRemembered(a));
   ASSERT_FALSE(tospace.Contains(*a.raw_ptr()));

   ASSERT_TRUE(SerialScavenge(heap));

   ASSERT_TRUE(a.raw_ptr()->IsRemembered());
   ASSERT_TRUE(IsNewWord(a, kAValue));
   ASSERT_TRUE(tospace.Contains((*a.raw_ptr())));

   ASSERT_NO_FATAL_FAILURE(LocalPage::SetForCurrentThread(nullptr));
 }

 TEST_F(SerialScavengerTest, TestSerialScavenge_WillPass_ScavengesMultipleObjects) {
   MemoryRegion page_region(LocalPage::CalculateLocalPageSize(32));
   ASSERT_TRUE(page_region.Protect(MemoryRegion::kReadWrite));
   LocalPage page(page_region);
   ASSERT_NO_FATAL_FAILURE(LocalPage::SetForCurrentThread(&page));

   MemoryRegion region(GetTotalInitialHeapSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto heap = Heap::From(region);
   auto zone = heap->new_zone();
   auto fromspace = zone->fromspace();
   auto tospace = zone->tospace();

   static constexpr const word kAValue = 33;
   Local<word> a(TryAllocateMarkedWord(zone, kAValue));
   DLOG(INFO) << "a: " << (*a.raw_ptr());
   ASSERT_TRUE(IsNewWord(a, kAValue));
   ASSERT_FALSE(IsRemembered(a));
   ASSERT_FALSE(tospace.Contains(*a.raw_ptr()));

   static constexpr const word kBValue = 66;
   Local<word> b(TryAllocateMarkedWord(zone, kBValue));
   DLOG(INFO) << "b: " << (*b.raw_ptr());
   ASSERT_TRUE(IsNewWord(b, kBValue));
   ASSERT_FALSE(IsRemembered(b));
   ASSERT_FALSE(tospace.Contains(*b.raw_ptr()));

   static constexpr const word kCValue = 99;
   Local<word> c(TryAllocateMarkedWord(zone, kCValue));
   DLOG(INFO) << "c: " << (*c.raw_ptr());
   ASSERT_TRUE(IsNewWord(c, kCValue));
   ASSERT_FALSE(IsRemembered(c));
   ASSERT_FALSE(tospace.Contains(*c.raw_ptr()));

   MockScavenger scavenger(heap);
   EXPECT_CALL(scavenger, Scavenge(IsPointerTo(a)))
       .WillOnce([&](Pointer* ptr) {
         DLOG(INFO) << "scavenging: " << (*ptr);
         auto new_ptr = TryAllocateWord(&tospace, kAValue);
         new_ptr->SetRememberedBit();
         return new_ptr->GetStartingAddress();
       });
   EXPECT_CALL(scavenger, Scavenge(IsPointerTo(b)))
       .WillOnce([&](Pointer* ptr) {
         DLOG(INFO) << "scavenging: " << (*ptr);
         auto new_ptr = TryAllocateWord(&tospace, kBValue);
         new_ptr->SetRememberedBit();
         return new_ptr->GetStartingAddress();
       });
   EXPECT_CALL(scavenger, Scavenge(IsPointerTo(c)))
       .WillOnce([&](Pointer* ptr) {
         DLOG(INFO) << "scavenging: " << (*ptr);
         auto new_ptr = TryAllocateWord(&tospace, kCValue);
         new_ptr->SetRememberedBit();
         return new_ptr->GetStartingAddress();
       });
   ASSERT_TRUE(SerialScavenge(&scavenger));

   ASSERT_TRUE(a.raw_ptr()->IsRemembered());
   ASSERT_TRUE(IsNewWord(a, kAValue));
   ASSERT_TRUE(tospace.Contains((*a.raw_ptr())));

   ASSERT_TRUE(b.raw_ptr()->IsRemembered());
   ASSERT_TRUE(IsNewWord(b, kBValue));
   ASSERT_TRUE(tospace.Contains((*b.raw_ptr())));

   ASSERT_TRUE(c.raw_ptr()->IsRemembered());
   ASSERT_TRUE(IsNewWord(c, kCValue));
   ASSERT_TRUE(tospace.Contains((*c.raw_ptr())));

   ASSERT_NO_FATAL_FAILURE(LocalPage::SetForCurrentThread(nullptr));
 }

 TEST_F(SerialScavengerTest, TestSerialScavenge_WillPass_PromotesOneObject) {
   MemoryRegion page_region(LocalPage::CalculateLocalPageSize(32));
   ASSERT_TRUE(page_region.Protect(MemoryRegion::kReadWrite));
   LocalPage page(page_region);
   ASSERT_NO_FATAL_FAILURE(LocalPage::SetForCurrentThread(&page));

   MemoryRegion region(GetTotalInitialHeapSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto heap = Heap::From(region);
   auto new_zone = heap->new_zone();
   auto fromspace = new_zone->fromspace();
   auto tospace = new_zone->tospace();

   auto old_zone = heap->old_zone();

   static constexpr const word kAValue = 33;
   Local<word> a(TryAllocateRememberedWord(new_zone, kAValue));
   a.raw_ptr()->SetMarkedBit();

   ASSERT_TRUE(IsNewWord(a, kAValue));
   ASSERT_TRUE(IsRemembered(a));
   ASSERT_FALSE(tospace.Contains(*a.raw_ptr()));

   MockScavenger scavenger(heap);
   EXPECT_CALL(scavenger, Promote(IsPointerTo(a)))
       .Times(1)
       .WillOnce([&](Pointer* ptr) {
         auto new_ptr = TryAllocateRememberedWord(old_zone, kAValue);
         DLOG(INFO) << "promoting " << (*ptr) << " to " << (*new_ptr);
         return new_ptr->GetStartingAddress();
       });

   ASSERT_TRUE(SerialScavenge(&scavenger));

   DLOG(INFO) << "new-zone: " << (*new_zone);
   DLOG(INFO) << "old-zone: " << (*old_zone);

   DLOG(INFO) << "a: " << (*a.raw_ptr());
   ASSERT_TRUE(a.raw_ptr()->IsRemembered());
   ASSERT_TRUE(IsOldWord(a, kAValue));
   ASSERT_TRUE(old_zone->Contains(*a.raw_ptr()));

   //TODO: ASSERT_FALSE(new_zone->Contains(*a.raw_ptr()));

   ASSERT_NO_FATAL_FAILURE(LocalPage::SetForCurrentThread(nullptr));
 }

 TEST_F(SerialScavengerTest, TestSerialScavenge_WillPass_PromotesMultipleObjects) {
   MemoryRegion page_region(LocalPage::CalculateLocalPageSize(32));
   ASSERT_TRUE(page_region.Protect(MemoryRegion::kReadWrite));
   LocalPage page(page_region);
   ASSERT_NO_FATAL_FAILURE(LocalPage::SetForCurrentThread(&page));

   MemoryRegion region(GetTotalInitialHeapSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto heap = Heap::From(region);
   auto new_zone = heap->new_zone();
   auto fromspace = new_zone->fromspace();
   auto tospace = new_zone->tospace();

   auto old_zone = heap->old_zone();

   static constexpr const word kAValue = 33;
   Local<word> a(TryAllocateRememberedWord(new_zone, kAValue));
   a.raw_ptr()->SetMarkedBit();
   ASSERT_TRUE(IsNewWord(a, kAValue));
   ASSERT_TRUE(IsRemembered(a));
   ASSERT_FALSE(tospace.Contains(*a.raw_ptr()));

   static constexpr const word kBValue = 66;
   Local<word> b(TryAllocateRememberedWord(new_zone, kBValue));
   b.raw_ptr()->SetMarkedBit();
   ASSERT_TRUE(IsNewWord(b, kBValue));
   ASSERT_TRUE(IsRemembered(b));
   ASSERT_FALSE(tospace.Contains(*b.raw_ptr()));

   static constexpr const word kCValue = 66;
   Local<word> c(TryAllocateRememberedWord(new_zone, kCValue));
   c.raw_ptr()->SetMarkedBit();
   ASSERT_TRUE(IsNewWord(c, kCValue));
   ASSERT_TRUE(IsRemembered(c));
   ASSERT_FALSE(tospace.Contains(*c.raw_ptr()));

   MockScavenger scavenger(heap);
   EXPECT_CALL(scavenger, Promote(IsPointerTo(a)))
       .WillOnce([&](Pointer* ptr) {
         DLOG(INFO) << "promoting " << (*ptr);
         auto new_ptr = TryAllocateRememberedWord(old_zone, kAValue);
         return new_ptr->GetStartingAddress();
       });
   EXPECT_CALL(scavenger, Promote(IsPointerTo(b)))
       .WillOnce([&](Pointer* ptr) {
         DLOG(INFO) << "promoting " << (*ptr);
         auto new_ptr = TryAllocateRememberedWord(old_zone, kBValue);
         return new_ptr->GetStartingAddress();
       });
   EXPECT_CALL(scavenger, Promote(IsPointerTo(c)))
       .WillOnce([&](Pointer* ptr) {
         DLOG(INFO) << "promoting " << (*ptr);
         auto new_ptr = TryAllocateRememberedWord(old_zone, kCValue);
         return new_ptr->GetStartingAddress();
       });

   ASSERT_TRUE(SerialScavenge(&scavenger));

   ASSERT_TRUE(a.raw_ptr()->IsRemembered());
   ASSERT_TRUE(IsOldWord(a, kAValue));
   ASSERT_TRUE(old_zone->Contains(*a.raw_ptr()));

   ASSERT_TRUE(b.raw_ptr()->IsRemembered());
   ASSERT_TRUE(IsOldWord(b, kBValue));
   ASSERT_TRUE(old_zone->Contains(*b.raw_ptr()));

   ASSERT_TRUE(c.raw_ptr()->IsRemembered());
   ASSERT_TRUE(IsOldWord(c, kCValue));
   ASSERT_TRUE(old_zone->Contains(*c.raw_ptr()));

   ASSERT_NO_FATAL_FAILURE(LocalPage::SetForCurrentThread(nullptr));
 }

 TEST_F(SerialScavengerTest, TestSerialScavenge_WillPass_ScavengesAndPromotesOneObject) {
   MemoryRegion page_region(LocalPage::CalculateLocalPageSize(32));
   ASSERT_TRUE(page_region.Protect(MemoryRegion::kReadWrite));
   LocalPage page(page_region);
   ASSERT_NO_FATAL_FAILURE(LocalPage::SetForCurrentThread(&page));

   MemoryRegion region(GetTotalInitialHeapSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto heap = Heap::From(region);
   auto new_zone = heap->new_zone();
   auto fromspace = new_zone->fromspace();
   auto tospace = new_zone->tospace();

   auto old_zone = heap->old_zone();

   static constexpr const word kAValue = 33;
   Local<word> a(TryAllocateWord(new_zone, kAValue));
   a.raw_ptr()->SetMarkedBit();
   ASSERT_TRUE(IsNewWord(a, kAValue));
   ASSERT_FALSE(IsRemembered(a));
   ASSERT_FALSE(tospace.Contains(*a.raw_ptr()));

   static constexpr const word kBValue = 66;
   Local<word> b(TryAllocateRememberedWord(new_zone, kBValue));
   b.raw_ptr()->SetMarkedBit();
   ASSERT_TRUE(IsNewWord(b, kBValue));
   ASSERT_TRUE(IsRemembered(b));
   ASSERT_FALSE(tospace.Contains(*b.raw_ptr()));

   MockScavenger scavenger(heap);
   EXPECT_CALL(scavenger, Scavenge(IsPointerTo(a)))
       .WillOnce([&](Pointer* ptr) {
         DLOG(INFO) << "scavenging " << (*ptr);
         auto new_ptr = TryAllocateRememberedWord(&tospace, kAValue);
         new_ptr->SetRememberedBit();
         return new_ptr->GetStartingAddress();
       });
   EXPECT_CALL(scavenger, Promote(IsPointerTo(b)))
       .WillOnce([&](Pointer* ptr) {
         DLOG(INFO) << "promoting " << (*ptr);
         auto new_ptr = TryAllocateRememberedWord(old_zone, kBValue);
         new_ptr->SetRememberedBit();
         return new_ptr->GetStartingAddress();
       });

   ASSERT_TRUE(SerialScavenge(&scavenger));

   ASSERT_TRUE(a.raw_ptr()->IsRemembered());
   ASSERT_TRUE(IsNewWord(a, kAValue));
   ASSERT_TRUE(tospace.Contains((*a.raw_ptr())));

   ASSERT_TRUE(b.raw_ptr()->IsRemembered());
   ASSERT_TRUE(IsOldWord(b, kBValue));
   ASSERT_TRUE(old_zone->Contains(*b.raw_ptr()));

   ASSERT_NO_FATAL_FAILURE(LocalPage::SetForCurrentThread(nullptr));
 }

 TEST_F(SerialScavengerTest, TestSerialScavenge_WillPass_ScavengesAndPromotesMultipleObjects) {
   MemoryRegion page_region(LocalPage::CalculateLocalPageSize(32));
   ASSERT_TRUE(page_region.Protect(MemoryRegion::kReadWrite));
   LocalPage page(page_region);
   ASSERT_NO_FATAL_FAILURE(LocalPage::SetForCurrentThread(&page));

   MemoryRegion region(GetTotalInitialHeapSize());
   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
   auto heap = Heap::From(region);
   auto new_zone = heap->new_zone();
   auto fromspace = new_zone->fromspace();
   auto tospace = new_zone->tospace();
   auto old_zone = heap->old_zone();

   static constexpr const word kAValue = 11;
   Local<word> a(TryAllocateWord(new_zone, kAValue));
   a.raw_ptr()->SetMarkedBit();
   DLOG(INFO) << "a: " << (*a.raw_ptr());
   ASSERT_TRUE(IsNewWord(a, kAValue));
   ASSERT_FALSE(IsRemembered(a));
   ASSERT_FALSE(tospace.Contains(*a.raw_ptr()));

   static constexpr const word kBValue = 22;
   Local<word> b(TryAllocateRememberedWord(new_zone, kBValue));
   b.raw_ptr()->SetMarkedBit();
   DLOG(INFO) << "b: " << (*b.raw_ptr());
   ASSERT_TRUE(IsNewWord(b, kBValue));
   ASSERT_TRUE(IsRemembered(b));
   ASSERT_FALSE(tospace.Contains(*b.raw_ptr()));

   static constexpr const word kCValue = 33;
   Local<word> c(TryAllocateWord(new_zone, kCValue));
   c.raw_ptr()->SetMarkedBit();
   DLOG(INFO) << "c: " << (*c.raw_ptr());
   ASSERT_TRUE(IsNewWord(c, kCValue));
   ASSERT_FALSE(IsRemembered(c));
   ASSERT_FALSE(tospace.Contains(*c.raw_ptr()));

   static constexpr const word kDValue = 44;
   Local<word> d(TryAllocateRememberedWord(new_zone, kDValue));
   d.raw_ptr()->SetMarkedBit();
   DLOG(INFO) << "d: " << (*d.raw_ptr());
   ASSERT_TRUE(IsNewWord(d, kDValue));
   ASSERT_TRUE(IsRemembered(d));
   ASSERT_FALSE(tospace.Contains(*d.raw_ptr()));

   static constexpr const word kEValue = 55;
   Local<word> e(TryAllocateWord(new_zone, kEValue));
   e.raw_ptr()->SetMarkedBit();
   DLOG(INFO) << "e: " << (*e.raw_ptr());
   ASSERT_TRUE(IsNewWord(e, kEValue));
   ASSERT_FALSE(IsRemembered(e));
   ASSERT_FALSE(tospace.Contains(*e.raw_ptr()));

   static constexpr const word kFValue = 66;
   Local<word> f(TryAllocateRememberedWord(new_zone, kFValue));
   f.raw_ptr()->SetMarkedBit();
   DLOG(INFO) << "f: " << (*f.raw_ptr());
   ASSERT_TRUE(IsNewWord(f, kFValue));
   ASSERT_TRUE(IsRemembered(f));
   ASSERT_FALSE(tospace.Contains(*f.raw_ptr()));

   MockScavenger scavenger(heap);
   EXPECT_CALL(scavenger, Scavenge(IsPointerTo(a)))
       .WillOnce([&](Pointer* ptr) {
         DLOG(INFO) << "scavenging: " << (*ptr);
         auto new_ptr = TryAllocateWord(&tospace, kAValue);
         new_ptr->SetRememberedBit();
         return new_ptr->GetStartingAddress();
       });
   EXPECT_CALL(scavenger, Promote(IsPointerTo(b)))
       .WillOnce([&](Pointer* ptr) {
         DLOG(INFO) << "promoting " << (*ptr);
         auto new_ptr = TryAllocateRememberedWord(old_zone, kBValue);
         new_ptr->SetRememberedBit();
         return new_ptr->GetStartingAddress();
       });
   EXPECT_CALL(scavenger, Scavenge(IsPointerTo(c)))
       .WillOnce([&](Pointer* ptr) {
         DLOG(INFO) << "scavenging: " << (*ptr);
         auto new_ptr = TryAllocateWord(&tospace, kCValue);
         new_ptr->SetRememberedBit();
         return new_ptr->GetStartingAddress();
       });
   EXPECT_CALL(scavenger, Promote(IsPointerTo(d)))
       .WillOnce([&](Pointer* ptr) {
         DLOG(INFO) << "promoting " << (*ptr);
         auto new_ptr = TryAllocateRememberedWord(old_zone, kDValue);
         new_ptr->SetRememberedBit();
         return new_ptr->GetStartingAddress();
       });
   EXPECT_CALL(scavenger, Scavenge(IsPointerTo(e)))
       .WillOnce([&](Pointer* ptr) {
         DLOG(INFO) << "scavenging: " << (*ptr);
         auto new_ptr = TryAllocateWord(&tospace, kEValue);
         new_ptr->SetRememberedBit();
         return new_ptr->GetStartingAddress();
       });
   EXPECT_CALL(scavenger, Promote(IsPointerTo(f)))
       .WillOnce([&](Pointer* ptr) {
         DLOG(INFO) << "promoting " << (*ptr);
         auto new_ptr = TryAllocateRememberedWord(old_zone, kFValue);
         return new_ptr->GetStartingAddress();
       });
   ASSERT_TRUE(SerialScavenge(&scavenger));

   ASSERT_TRUE(a.raw_ptr()->IsRemembered());
   ASSERT_TRUE(IsNewWord(a, kAValue));
   ASSERT_TRUE(tospace.Contains((*a.raw_ptr())));

   ASSERT_TRUE(b.raw_ptr()->IsRemembered());
   ASSERT_TRUE(IsOldWord(b, kBValue));
   ASSERT_TRUE(old_zone->Contains(*b.raw_ptr()));

   ASSERT_TRUE(c.raw_ptr()->IsRemembered());
   ASSERT_TRUE(IsNewWord(c, kCValue));
   ASSERT_TRUE(tospace.Contains((*c.raw_ptr())));

   ASSERT_TRUE(d.raw_ptr()->IsRemembered());
   ASSERT_TRUE(IsOldWord(d, kDValue));
   ASSERT_TRUE(old_zone->Contains(*d.raw_ptr()));

   ASSERT_TRUE(e.raw_ptr()->IsRemembered());
   ASSERT_TRUE(IsNewWord(e, kEValue));
   ASSERT_TRUE(tospace.Contains((*e.raw_ptr())));

   ASSERT_TRUE(f.raw_ptr()->IsRemembered());
   ASSERT_TRUE(IsOldWord(f, kFValue));
   ASSERT_TRUE(old_zone->Contains(*f.raw_ptr()));

   ASSERT_NO_FATAL_FAILURE(LocalPage::SetForCurrentThread(nullptr));
 }
}