#include <gtest/gtest.h>
#include "poseidon/type.h"

namespace poseidon {
 class UInt8Test : public ::testing::Test {
  protected:
   MemoryRegion region_;
   NewZone zone_;

   UInt8Test():
    region_(flags::GetNewZoneSize(), MemoryRegion::kReadWrite),
    zone_(region_) {
   }

   inline MemoryRegion& region() {
     return region_;
   }

   inline NewZone& zone() {
     return zone_;
   }
  public:
   ~UInt8Test() override = default;

   void SetUp() override {
     // do something?
   }

   void TearDown() override {
     // do something?
   }
 };

 TEST_F(UInt8Test, TestNew_WillPass) {
   static constexpr const RawUInt8 kRawValue1 = 33;
   auto value = UInt8::TryAllocateIn(&zone(), kRawValue1);
   ASSERT_NE(value, nullptr);
   ASSERT_EQ(value->Get(), kRawValue1);
   static constexpr const RawUInt8 kRawValue2 = 99;
   ASSERT_NO_FATAL_FAILURE(value->Set(kRawValue2));
   ASSERT_EQ(value->Get(), kRawValue2);
   DLOG(INFO) << "value: " << (*value->raw_ptr());
 }
}