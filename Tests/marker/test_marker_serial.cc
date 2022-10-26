#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "helpers.h"
#include "poseidon/flags.h"
#include "poseidon/heap/new_page.h"
#include "poseidon/marker/marker_serial.h"

namespace poseidon {
#define UNALLOCATED 0 //TODO: cleanup

 using namespace ::testing;

 class SerialMarkerTest : public Test {
  protected:
   SerialMarkerTest() = default;

   static inline uword
   TryAllocateBytes(NewPage& page, const ObjectSize size) {
     return page.TryAllocate(size);
   }

   static inline RawObject*
   TryAllocateWord(NewPage& page, word value) {
     auto address = TryAllocateBytes(page, kWordSize);
     if (address == UNALLOCATED)
       return nullptr;
     auto ptr = (RawObject*)address;
     (*((word*)ptr->GetObjectPointerAddress())) = value;
     return ptr;
   }

   static inline RawObject*
   TryAllocateMarkedWord(NewPage& page, word value) {
     auto address = TryAllocateBytes(page, kWordSize);
     if (address == UNALLOCATED)
       return nullptr;
     auto ptr = (RawObject*)address;
     ptr->SetMarkedBit();
     (*((word*)ptr->GetObjectPointerAddress())) = value;
     return ptr;
   }
  public:
   ~SerialMarkerTest() override = default;
 };

 TEST_F(SerialMarkerTest, TestMark) {
   MemoryRegion region(GetNewPageSize());
   NewPage page(0, region);

   static const constexpr word kAValue = 10;
   auto a = TryAllocateWord(page, kAValue);
   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsNew(a));
   ASSERT_FALSE(IsMarked(a));
   ASSERT_TRUE(IsWord(a, kAValue));
   ASSERT_TRUE(page.Contains(*a));

   static const constexpr word kBValue = 33;
   auto b = TryAllocateWord(page, kBValue);
   ASSERT_TRUE(IsAllocated(b));
   ASSERT_TRUE(IsNew(b));
   ASSERT_FALSE(IsMarked(b));
   ASSERT_TRUE(IsWord(b, kBValue));
   ASSERT_TRUE(page.Contains(*b));

   static const constexpr word kCValue = 33;
   auto c = TryAllocateWord(page, kCValue);
   ASSERT_TRUE(IsAllocated(c));
   ASSERT_TRUE(IsNew(c));
   ASSERT_FALSE(IsMarked(c));
   ASSERT_TRUE(IsWord(c, kCValue));
   ASSERT_TRUE(page.Contains(*c));

   SerialMarker marker;
   ASSERT_TRUE(marker.MarkPage(&page));

   ASSERT_TRUE(IsAllocated(a));
   ASSERT_TRUE(IsNew(a));
   ASSERT_TRUE(IsMarked(a));
   ASSERT_TRUE(IsWord(a, kAValue));
   ASSERT_TRUE(page.Contains(*a));

   ASSERT_TRUE(IsAllocated(b));
   ASSERT_TRUE(IsNew(b));
   ASSERT_TRUE(IsMarked(b));
   ASSERT_TRUE(IsWord(b, kBValue));
   ASSERT_TRUE(page.Contains(*b));

   ASSERT_TRUE(IsAllocated(c));
   ASSERT_TRUE(IsNew(c));
   ASSERT_TRUE(IsMarked(c));
   ASSERT_TRUE(IsWord(c, kCValue));
   ASSERT_TRUE(page.Contains(*c));
 }
}