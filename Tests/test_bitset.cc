#include <gtest/gtest.h>

#include "poseidon/heap.h"
#include "poseidon/bitset.h"

namespace poseidon{
 using namespace ::testing;

 static constexpr const int64_t kB = 1;
 static constexpr const int64_t kKB = 1024 * kB;
 static constexpr const int64_t kMB = 1024 * kKB;

 TEST(BitSetTest, Test){
   BitSet table(4096);
   table.Set(1024);
   table.Set(2048);
   table.Set(2049);
   DLOG(INFO) << "bits: " << std::endl << table;

   ASSERT_FALSE(table.Get(256));
   ASSERT_FALSE(table.Get(1023));
   ASSERT_TRUE(table.Get(1024));
   ASSERT_FALSE(table.Get(1025));
   ASSERT_FALSE(table.Get(2047));
   ASSERT_TRUE(table.Get(2048));
   ASSERT_TRUE(table.Get(2049));
   ASSERT_FALSE(table.Get(2050));
   ASSERT_FALSE(table.Get(4096));
 }
}