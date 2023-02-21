#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "matchers/is_pointer_to.h"
#include "scavenger/mock_scavenger.h"
#include "assertions/ptr_assertions.h"
#include "assertions/type_assertions.h"
#include "poseidon/marker/marker.h"
#include "poseidon/scavenger/scavenger_serial.h"

namespace poseidon {
 using namespace ::testing;

 class SerialScavengerTest : public Test {
  protected:
   MemoryRegion region_;
   NewZone zone_;
   Marker marker_;

   SerialScavengerTest():
    region_(NewZone::GetNewZoneSize(), MemoryRegion::kReadWrite),
    zone_(region_) {
   }

   inline MemoryRegion& region() {
     return region_;
   }

   inline NewZone& zone() {
     return zone_;
   }

   inline void SerialScavenge(MockScavenger* scavenger) {
     SerialScavenger serial_scavenger(scavenger, &zone(), nullptr);
     return serial_scavenger.ScavengeMemory();
   }

   inline void Mark(Pointer* ptr) {
     ptr->SetMarked();
   }

   template<class T>
   inline void Mark(Local<T>& local) {
     return Mark(local.raw_ptr());
   }
  public:
   ~SerialScavengerTest() override = default;

   void SetUp() override {
     ASSERT_NO_FATAL_FAILURE(zone().SetWritable());
     ASSERT_NO_FATAL_FAILURE(zone().Clear());
   }

   void TearDown() override {
     ASSERT_NO_FATAL_FAILURE(zone().SetReadOnly());
   }
 };

 TEST_F(SerialScavengerTest, TestSerialScavenge_WillFail_NoRootSet) {
   MockScavenger scavenger(&zone(), nullptr);
   ASSERT_DEATH(SerialScavenge(&scavenger), "no local page exists for current thread.");
 }

 TEST_F(SerialScavengerTest, TestSerialScavenge_WillPass_DoesNothing) {
   LocalScope local_scope;

   auto fromspace = zone().fromspace();
   auto tospace = zone().tospace();

   static constexpr const RawInt32 kAValue = 33;
   auto a = Int32::TryAllocateIn(&zone(), kAValue);
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsInt32(a->raw_ptr()));
   ASSERT_TRUE(Int32Eq(kAValue, a));

   MockScavenger scavenger(&zone(), nullptr);
   ASSERT_NO_FATAL_FAILURE(SerialScavenge(&scavenger));

   // scavenging should always flip the semi-spaces
   ASSERT_EQ(tospace, zone().fromspace());
 }

 TEST_F(SerialScavengerTest, TestSerialScavenge_WillPass_ScavengesOneInt32) {
   LocalScope local_scope;

   auto fromspace = zone().GetFromspace();
   auto tospace = zone().GetTospace();

   static constexpr const RawInt32 kAValue = 33;
   Local<Int32> a(Int32::TryAllocateIn(&zone(), kAValue));
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsInt32(a));
   ASSERT_TRUE(Int32Eq(kAValue, a));
   ASSERT_FALSE(IsMarked(a));
   ASSERT_NO_FATAL_FAILURE(Mark(a));
   ASSERT_TRUE(IsMarked(a));
   ASSERT_FALSE(IsRemembered(a));
   ASSERT_TRUE(fromspace.Contains((const Region&)*a->raw_ptr()));
   ASSERT_FALSE(tospace.Contains((const Region&)*a->raw_ptr()));

   MockScavenger scavenger(&zone(), nullptr);
   EXPECT_CALL(scavenger, Scavenge(IsPointerTo(a)));
   ASSERT_NO_FATAL_FAILURE(SerialScavenge(&scavenger));

   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsInt32(a));
   ASSERT_TRUE(Int32Eq(kAValue, a));
   ASSERT_FALSE(IsMarked(a));
   ASSERT_TRUE(IsRemembered(a));
   ASSERT_FALSE(fromspace.Contains((const Region&)*a.raw_ptr()));
   ASSERT_TRUE(tospace.Contains((const Region&)*a.raw_ptr()));
 }

 TEST_F(SerialScavengerTest, TestSerialScavenge_WillPass_ScavengesMultipleInt32s) {
   LocalScope local_scope;
   auto fromspace = zone().GetFromspace();
   auto tospace = zone().GetTospace();

   static constexpr const RawInt32 kAValue = 33;
   Local<Int32> a(Int32::TryAllocateIn(&zone(), kAValue));
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsInt32(a));
   ASSERT_TRUE(Int32Eq(kAValue, a));
   ASSERT_NO_FATAL_FAILURE(Mark(a));
   ASSERT_TRUE(IsMarked(a));
   ASSERT_FALSE(IsRemembered(a));
   ASSERT_TRUE(fromspace.Contains((const Region&)*a.raw_ptr()));
   ASSERT_FALSE(tospace.Contains((const Region&)*a.raw_ptr()));
   DLOG(INFO) << "a (before): " << a;

   static constexpr const RawInt32 kBValue = 33;
   Local<Int32> b(Int32::TryAllocateIn(&zone(), kBValue));
   ASSERT_TRUE(IsAllocated(b));
   ASSERT_TRUE(IsInt32(b));
   ASSERT_TRUE(Int32Eq(kBValue, b));
   ASSERT_NO_FATAL_FAILURE(Mark(b));
   ASSERT_TRUE(IsMarked(b));
   ASSERT_FALSE(IsRemembered(b));
   ASSERT_TRUE(fromspace.Contains((const Region&)*b.raw_ptr()));
   ASSERT_FALSE(tospace.Contains((const Region&)*b.raw_ptr()));
   DLOG(INFO) << "b (before): " << b;

   MockScavenger scavenger(&zone(), nullptr);
   EXPECT_CALL(scavenger, Scavenge(IsPointerTo(a)));
   EXPECT_CALL(scavenger, Scavenge(IsPointerTo(b)));
   ASSERT_NO_FATAL_FAILURE(SerialScavenge(&scavenger));

   ASSERT_FALSE(IsMarked(a));
   ASSERT_TRUE(IsRemembered(a));
   ASSERT_TRUE(IsInt32(a));
   ASSERT_TRUE(Int32Eq(kAValue, a));
   ASSERT_FALSE(fromspace.Contains((const Region&)*a.raw_ptr()));
   ASSERT_TRUE(tospace.Contains((const Region&)*a.raw_ptr()));
   DLOG(INFO) << "a (after): " << a;

   ASSERT_FALSE(b.raw_ptr()->IsMarked());
   ASSERT_TRUE(b.raw_ptr()->IsRemembered());
   ASSERT_TRUE(IsInt32(b.raw_ptr()));
   ASSERT_TRUE(Int32Eq(kAValue, b.Get()));
   ASSERT_FALSE(fromspace.Contains((const Region&)*b.raw_ptr()));
   ASSERT_TRUE(tospace.Contains((const Region&)*b.raw_ptr()));
   DLOG(INFO) << "b (after): " << b;
 }

 TEST_F(SerialScavengerTest, TestSerialScavenge_WillPass_ScavengesOneTuple) {
   LocalScope local_scope;
   auto fromspace = zone().GetFromspace();
   auto tospace = zone().GetTospace();

   static constexpr const RawInt32 kAValue = 33;
   auto a = Int32::TryAllocateIn(&zone(), kAValue);
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsInt32(a));
   ASSERT_TRUE(Int32Eq(kAValue, a));
   ASSERT_FALSE(IsMarked(a));
   ASSERT_FALSE(IsRemembered(a));
   ASSERT_TRUE(fromspace.Contains((const Region&)*a->raw_ptr()));
   ASSERT_FALSE(tospace.Contains((const Region&)*a->raw_ptr()));

   static constexpr const RawInt32 kBValue = 66;
   auto b = Int32::TryAllocateIn(&zone(), kBValue);
   ASSERT_TRUE(IsAllocated(b));
   ASSERT_TRUE(IsInt32(b));
   ASSERT_TRUE(Int32Eq(kBValue, b));
   ASSERT_FALSE(IsMarked(b));
   ASSERT_FALSE(IsRemembered(b));
   ASSERT_TRUE(fromspace.Contains((const Region&)*b->raw_ptr()));
   ASSERT_FALSE(tospace.Contains((const Region&)*b->raw_ptr()));

   Local<Tuple> c(Tuple::TryAllocateIn(&zone()));
   ASSERT_TRUE(IsAllocated(c));
   ASSERT_TRUE(IsTuple(c));
   ASSERT_TRUE(IsUnallocated(c->GetCarPointer()));
   ASSERT_TRUE(IsUnallocated(c->GetCdrPointer()));
   ASSERT_NO_FATAL_FAILURE(c->SetCar(a));
   ASSERT_TRUE(IsAllocated(c->GetCarPointer()));
   ASSERT_TRUE(IsInt32(c->GetCarPointer()));
   ASSERT_TRUE(Int32Eq(a, c->GetCar<Int32>()));
   ASSERT_NO_FATAL_FAILURE(c->SetCdr(b));
   ASSERT_TRUE(IsAllocated(c->GetCdrPointer()));
   ASSERT_TRUE(IsInt32(c->GetCdrPointer()));
   ASSERT_TRUE(Int32Eq(b, c->GetCdr<Int32>()));
   ASSERT_FALSE(IsMarked(c));
   ASSERT_NO_FATAL_FAILURE(Mark(c));
   ASSERT_TRUE(IsMarked(c));
   ASSERT_FALSE(IsRemembered(c));
   ASSERT_TRUE(fromspace.Contains((const Region&)*c->raw_ptr()));
   ASSERT_FALSE(tospace.Contains((const Region&)*c->raw_ptr()));

   MockScavenger scavenger(&zone(), nullptr);
   EXPECT_CALL(scavenger, Scavenge(IsPointerTo(c)));
   ASSERT_NO_FATAL_FAILURE(SerialScavenge(&scavenger));

   ASSERT_TRUE(IsAllocated(c));
   ASSERT_TRUE(IsTuple(c));
   ASSERT_FALSE(IsMarked(c));
   ASSERT_TRUE(IsRemembered(c));
   ASSERT_FALSE(fromspace.Contains((const Region&)*c->raw_ptr()));
   ASSERT_TRUE(tospace.Contains((const Region&)*c->raw_ptr()));

   ASSERT_TRUE(IsAllocated(c->GetCarPointer()));
   ASSERT_TRUE(IsInt32(c->GetCarPointer()));
   ASSERT_TRUE(Int32Eq(a, c->GetCar<Int32>()));

   ASSERT_TRUE(IsAllocated(c->GetCdrPointer()));
   ASSERT_TRUE(IsInt32(c->GetCdrPointer()));
   ASSERT_TRUE(Int32Eq(b, c->GetCdr<Int32>()));

   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsInt32(a));
   ASSERT_TRUE(Int32Eq(kAValue, a));
   ASSERT_FALSE(IsMarked(a));
   ASSERT_TRUE(IsRemembered(a));
   ASSERT_FALSE(fromspace.Contains((const Region&)*a->raw_ptr()));
   ASSERT_TRUE(tospace.Contains((const Region&)*a->raw_ptr()));

   ASSERT_TRUE(IsAllocated(b));
   ASSERT_TRUE(IsInt32(b));
   ASSERT_TRUE(Int32Eq(kBValue, b));
   ASSERT_FALSE(IsMarked(b));
   ASSERT_TRUE(IsRemembered(b));
   ASSERT_FALSE(fromspace.Contains((const Region&)*b->raw_ptr()));
   ASSERT_TRUE(tospace.Contains((const Region&)*b->raw_ptr()));
 }

// TEST_F(SerialScavengerTest, TestSerialScavenge_WillPass_PromotesOneObject) {
//   MemoryRegion page_region(LocalPage::CalculateLocalPageSize(32));
//   ASSERT_TRUE(page_region.Protect(MemoryRegion::kReadWrite));
//   LocalPage page(page_region);
//   ASSERT_NO_FATAL_FAILURE(LocalPage::SetForCurrentThread(&page));
//
//   MemoryRegion region(flags::GetTotalInitialHeapSize());
//   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
//   auto heap = Heap::From(region);
//   auto new_zone = heap->new_zone();
//   auto fromspace = new_zone->fromspace();
//   auto tospace = new_zone->tospace();
//
//   auto old_zone = heap->old_zone();
//
//   static constexpr const word kAValue = 33;
//   Local<word> a(TryAllocateRememberedWord(new_zone, kAValue));
//   a.raw_ptr()->SetMarkedBit();
//
//   ASSERT_TRUE(IsNewWord(a, kAValue));
//   ASSERT_TRUE(IsRemembered(a));
//   ASSERT_FALSE(tospace.Intersects(*a.raw_ptr()));
//
//   MockScavenger scavenger(heap);
//   EXPECT_CALL(scavenger, Promote(IsPointerTo(a)))
//       .Times(1)
//       .WillOnce([&](Pointer* ptr) {
//         auto new_ptr = TryAllocateRememberedWord(old_zone, kAValue);
//         DLOG(INFO) << "promoting " << (*ptr) << " to " << (*new_ptr);
//         return new_ptr->GetStartingAddress();
//       });
//
//   ASSERT_TRUE(SerialScavenge(&scavenger));
//
//   DLOG(INFO) << "new-zone: " << (*new_zone);
//   DLOG(INFO) << "old-zone: " << (*old_zone);
//
//   DLOG(INFO) << "a: " << (*a.raw_ptr());
//   ASSERT_TRUE(a.raw_ptr()->IsRemembered());
//   ASSERT_TRUE(IsOldWord(a, kAValue));
//   ASSERT_TRUE(old_zone->Intersects(*a.raw_ptr()));
//
//   //TODO: ASSERT_FALSE(new_zone->Intersects(*a.raw_ptr()));
//
//   ASSERT_NO_FATAL_FAILURE(LocalPage::SetForCurrentThread(nullptr));
// }
//
// TEST_F(SerialScavengerTest, TestSerialScavenge_WillPass_PromotesMultipleObjects) {
//   MemoryRegion page_region(LocalPage::CalculateLocalPageSize(32));
//   ASSERT_TRUE(page_region.Protect(MemoryRegion::kReadWrite));
//   LocalPage page(page_region);
//   ASSERT_NO_FATAL_FAILURE(LocalPage::SetForCurrentThread(&page));
//
//   MemoryRegion region(flags::GetTotalInitialHeapSize());
//   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
//   auto heap = Heap::From(region);
//   auto new_zone = heap->new_zone();
//   auto fromspace = new_zone->fromspace();
//   auto tospace = new_zone->tospace();
//
//   auto old_zone = heap->old_zone();
//
//   static constexpr const word kAValue = 33;
//   Local<word> a(TryAllocateRememberedWord(new_zone, kAValue));
//   a.raw_ptr()->SetMarkedBit();
//   ASSERT_TRUE(IsNewWord(a, kAValue));
//   ASSERT_TRUE(IsRemembered(a));
//   ASSERT_FALSE(tospace.Intersects(*a.raw_ptr()));
//
//   static constexpr const word kBValue = 66;
//   Local<word> b(TryAllocateRememberedWord(new_zone, kBValue));
//   b.raw_ptr()->SetMarkedBit();
//   ASSERT_TRUE(IsNewWord(b, kBValue));
//   ASSERT_TRUE(IsRemembered(b));
//   ASSERT_FALSE(tospace.Intersects(*b.raw_ptr()));
//
//   static constexpr const word kCValue = 66;
//   Local<word> c(TryAllocateRememberedWord(new_zone, kCValue));
//   c.raw_ptr()->SetMarkedBit();
//   ASSERT_TRUE(IsNewWord(c, kCValue));
//   ASSERT_TRUE(IsRemembered(c));
//   ASSERT_FALSE(tospace.Intersects(*c.raw_ptr()));
//
//   MockScavenger scavenger(heap);
//   EXPECT_CALL(scavenger, Promote(IsPointerTo(a)))
//       .WillOnce([&](Pointer* ptr) {
//         DLOG(INFO) << "promoting " << (*ptr);
//         auto new_ptr = TryAllocateRememberedWord(old_zone, kAValue);
//         return new_ptr->GetStartingAddress();
//       });
//   EXPECT_CALL(scavenger, Promote(IsPointerTo(b)))
//       .WillOnce([&](Pointer* ptr) {
//         DLOG(INFO) << "promoting " << (*ptr);
//         auto new_ptr = TryAllocateRememberedWord(old_zone, kBValue);
//         return new_ptr->GetStartingAddress();
//       });
//   EXPECT_CALL(scavenger, Promote(IsPointerTo(c)))
//       .WillOnce([&](Pointer* ptr) {
//         DLOG(INFO) << "promoting " << (*ptr);
//         auto new_ptr = TryAllocateRememberedWord(old_zone, kCValue);
//         return new_ptr->GetStartingAddress();
//       });
//
//   ASSERT_TRUE(SerialScavenge(&scavenger));
//
//   ASSERT_TRUE(a.raw_ptr()->IsRemembered());
//   ASSERT_TRUE(IsOldWord(a, kAValue));
//   ASSERT_TRUE(old_zone->Intersects(*a.raw_ptr()));
//
//   ASSERT_TRUE(b.raw_ptr()->IsRemembered());
//   ASSERT_TRUE(IsOldWord(b, kBValue));
//   ASSERT_TRUE(old_zone->Intersects(*b.raw_ptr()));
//
//   ASSERT_TRUE(c.raw_ptr()->IsRemembered());
//   ASSERT_TRUE(IsOldWord(c, kCValue));
//   ASSERT_TRUE(old_zone->Intersects(*c.raw_ptr()));
//
//   ASSERT_NO_FATAL_FAILURE(LocalPage::SetForCurrentThread(nullptr));
// }
//
// TEST_F(SerialScavengerTest, TestSerialScavenge_WillPass_ScavengesAndPromotesOneObject) {
//   MemoryRegion page_region(LocalPage::CalculateLocalPageSize(32));
//   ASSERT_TRUE(page_region.Protect(MemoryRegion::kReadWrite));
//   LocalPage page(page_region);
//   ASSERT_NO_FATAL_FAILURE(LocalPage::SetForCurrentThread(&page));
//
//   MemoryRegion region(flags::GetTotalInitialHeapSize());
//   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
//   auto heap = Heap::From(region);
//   auto new_zone = heap->new_zone();
//   auto fromspace = new_zone->fromspace();
//   auto tospace = new_zone->tospace();
//
//   auto old_zone = heap->old_zone();
//
//   static constexpr const word kAValue = 33;
//   Local<word> a(TryAllocateWord(new_zone, kAValue));
//   a.raw_ptr()->SetMarkedBit();
//   ASSERT_TRUE(IsNewWord(a, kAValue));
//   ASSERT_FALSE(IsRemembered(a));
//   ASSERT_FALSE(tospace.Intersects(*a.raw_ptr()));
//
//   static constexpr const word kBValue = 66;
//   Local<word> b(TryAllocateRememberedWord(new_zone, kBValue));
//   b.raw_ptr()->SetMarkedBit();
//   ASSERT_TRUE(IsNewWord(b, kBValue));
//   ASSERT_TRUE(IsRemembered(b));
//   ASSERT_FALSE(tospace.Intersects(*b.raw_ptr()));
//
//   MockScavenger scavenger(heap);
//   EXPECT_CALL(scavenger, Scavenge(IsPointerTo(a)))
//       .WillOnce([&](Pointer* ptr) {
//         DLOG(INFO) << "scavenging " << (*ptr);
//         auto new_ptr = TryAllocateRememberedWord(&tospace, kAValue);
//         new_ptr->SetRememberedBit();
//         return new_ptr->GetStartingAddress();
//       });
//   EXPECT_CALL(scavenger, Promote(IsPointerTo(b)))
//       .WillOnce([&](Pointer* ptr) {
//         DLOG(INFO) << "promoting " << (*ptr);
//         auto new_ptr = TryAllocateRememberedWord(old_zone, kBValue);
//         new_ptr->SetRememberedBit();
//         return new_ptr->GetStartingAddress();
//       });
//
//   ASSERT_TRUE(SerialScavenge(&scavenger));
//
//   ASSERT_TRUE(a.raw_ptr()->IsRemembered());
//   ASSERT_TRUE(IsNewWord(a, kAValue));
//   ASSERT_TRUE(tospace.Intersects((*a.raw_ptr())));
//
//   ASSERT_TRUE(b.raw_ptr()->IsRemembered());
//   ASSERT_TRUE(IsOldWord(b, kBValue));
//   ASSERT_TRUE(old_zone->Intersects(*b.raw_ptr()));
//
//   ASSERT_NO_FATAL_FAILURE(LocalPage::SetForCurrentThread(nullptr));
// }
//
// TEST_F(SerialScavengerTest, TestSerialScavenge_WillPass_ScavengesAndPromotesMultipleObjects) {
//   MemoryRegion page_region(LocalPage::CalculateLocalPageSize(32));
//   ASSERT_TRUE(page_region.Protect(MemoryRegion::kReadWrite));
//   LocalPage page(page_region);
//   ASSERT_NO_FATAL_FAILURE(LocalPage::SetForCurrentThread(&page));
//
//   MemoryRegion region(flags::GetTotalInitialHeapSize());
//   ASSERT_TRUE(region.Protect(MemoryRegion::kReadWrite));
//   auto heap = Heap::From(region);
//   auto new_zone = heap->new_zone();
//   auto fromspace = new_zone->fromspace();
//   auto tospace = new_zone->tospace();
//   auto old_zone = heap->old_zone();
//
//   static constexpr const word kAValue = 11;
//   Local<word> a(TryAllocateWord(new_zone, kAValue));
//   a.raw_ptr()->SetMarkedBit();
//   DLOG(INFO) << "a: " << (*a.raw_ptr());
//   ASSERT_TRUE(IsNewWord(a, kAValue));
//   ASSERT_FALSE(IsRemembered(a));
//   ASSERT_FALSE(tospace.Intersects(*a.raw_ptr()));
//
//   static constexpr const word kBValue = 22;
//   Local<word> b(TryAllocateRememberedWord(new_zone, kBValue));
//   b.raw_ptr()->SetMarkedBit();
//   DLOG(INFO) << "b: " << (*b.raw_ptr());
//   ASSERT_TRUE(IsNewWord(b, kBValue));
//   ASSERT_TRUE(IsRemembered(b));
//   ASSERT_FALSE(tospace.Intersects(*b.raw_ptr()));
//
//   static constexpr const word kCValue = 33;
//   Local<word> c(TryAllocateWord(new_zone, kCValue));
//   c.raw_ptr()->SetMarkedBit();
//   DLOG(INFO) << "c: " << (*c.raw_ptr());
//   ASSERT_TRUE(IsNewWord(c, kCValue));
//   ASSERT_FALSE(IsRemembered(c));
//   ASSERT_FALSE(tospace.Intersects(*c.raw_ptr()));
//
//   static constexpr const word kDValue = 44;
//   Local<word> d(TryAllocateRememberedWord(new_zone, kDValue));
//   d.raw_ptr()->SetMarkedBit();
//   DLOG(INFO) << "d: " << (*d.raw_ptr());
//   ASSERT_TRUE(IsNewWord(d, kDValue));
//   ASSERT_TRUE(IsRemembered(d));
//   ASSERT_FALSE(tospace.Intersects(*d.raw_ptr()));
//
//   static constexpr const word kEValue = 55;
//   Local<word> e(TryAllocateWord(new_zone, kEValue));
//   e.raw_ptr()->SetMarkedBit();
//   DLOG(INFO) << "e: " << (*e.raw_ptr());
//   ASSERT_TRUE(IsNewWord(e, kEValue));
//   ASSERT_FALSE(IsRemembered(e));
//   ASSERT_FALSE(tospace.Intersects(*e.raw_ptr()));
//
//   static constexpr const word kFValue = 66;
//   Local<word> f(TryAllocateRememberedWord(new_zone, kFValue));
//   f.raw_ptr()->SetMarkedBit();
//   DLOG(INFO) << "f: " << (*f.raw_ptr());
//   ASSERT_TRUE(IsNewWord(f, kFValue));
//   ASSERT_TRUE(IsRemembered(f));
//   ASSERT_FALSE(tospace.Intersects(*f.raw_ptr()));
//
//   MockScavenger scavenger(heap);
//   EXPECT_CALL(scavenger, Scavenge(IsPointerTo(a)))
//       .WillOnce([&](Pointer* ptr) {
//         DLOG(INFO) << "scavenging: " << (*ptr);
//         auto new_ptr = TryAllocateWord(&tospace, kAValue);
//         new_ptr->SetRememberedBit();
//         return new_ptr->GetStartingAddress();
//       });
//   EXPECT_CALL(scavenger, Promote(IsPointerTo(b)))
//       .WillOnce([&](Pointer* ptr) {
//         DLOG(INFO) << "promoting " << (*ptr);
//         auto new_ptr = TryAllocateRememberedWord(old_zone, kBValue);
//         new_ptr->SetRememberedBit();
//         return new_ptr->GetStartingAddress();
//       });
//   EXPECT_CALL(scavenger, Scavenge(IsPointerTo(c)))
//       .WillOnce([&](Pointer* ptr) {
//         DLOG(INFO) << "scavenging: " << (*ptr);
//         auto new_ptr = TryAllocateWord(&tospace, kCValue);
//         new_ptr->SetRememberedBit();
//         return new_ptr->GetStartingAddress();
//       });
//   EXPECT_CALL(scavenger, Promote(IsPointerTo(d)))
//       .WillOnce([&](Pointer* ptr) {
//         DLOG(INFO) << "promoting " << (*ptr);
//         auto new_ptr = TryAllocateRememberedWord(old_zone, kDValue);
//         new_ptr->SetRememberedBit();
//         return new_ptr->GetStartingAddress();
//       });
//   EXPECT_CALL(scavenger, Scavenge(IsPointerTo(e)))
//       .WillOnce([&](Pointer* ptr) {
//         DLOG(INFO) << "scavenging: " << (*ptr);
//         auto new_ptr = TryAllocateWord(&tospace, kEValue);
//         new_ptr->SetRememberedBit();
//         return new_ptr->GetStartingAddress();
//       });
//   EXPECT_CALL(scavenger, Promote(IsPointerTo(f)))
//       .WillOnce([&](Pointer* ptr) {
//         DLOG(INFO) << "promoting " << (*ptr);
//         auto new_ptr = TryAllocateRememberedWord(old_zone, kFValue);
//         return new_ptr->GetStartingAddress();
//       });
//   ASSERT_TRUE(SerialScavenge(&scavenger));
//
//   ASSERT_TRUE(a.raw_ptr()->IsRemembered());
//   ASSERT_TRUE(IsNewWord(a, kAValue));
//   ASSERT_TRUE(tospace.Intersects((*a.raw_ptr())));
//
//   ASSERT_TRUE(b.raw_ptr()->IsRemembered());
//   ASSERT_TRUE(IsOldWord(b, kBValue));
//   ASSERT_TRUE(old_zone->Intersects(*b.raw_ptr()));
//
//   ASSERT_TRUE(c.raw_ptr()->IsRemembered());
//   ASSERT_TRUE(IsNewWord(c, kCValue));
//   ASSERT_TRUE(tospace.Intersects((*c.raw_ptr())));
//
//   ASSERT_TRUE(d.raw_ptr()->IsRemembered());
//   ASSERT_TRUE(IsOldWord(d, kDValue));
//   ASSERT_TRUE(old_zone->Intersects(*d.raw_ptr()));
//
//   ASSERT_TRUE(e.raw_ptr()->IsRemembered());
//   ASSERT_TRUE(IsNewWord(e, kEValue));
//   ASSERT_TRUE(tospace.Intersects((*e.raw_ptr())));
//
//   ASSERT_TRUE(f.raw_ptr()->IsRemembered());
//   ASSERT_TRUE(IsOldWord(f, kFValue));
//   ASSERT_TRUE(old_zone->Intersects(*f.raw_ptr()));
//
//   ASSERT_NO_FATAL_FAILURE(LocalPage::SetForCurrentThread(nullptr));
// }
}