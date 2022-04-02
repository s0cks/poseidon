#include <gtest/gtest.h>


#include "poseidon/bitset.h"

namespace poseidon{
 using namespace ::testing;

 TEST(BitSetTest, Test){
   BitSet table(4096);
   table.Set(1024, true);
   table.Set(2048, true);
   table.Set(2049, true);

   ASSERT_FALSE(table.Test(256));
   ASSERT_FALSE(table.Test(1023));
   ASSERT_TRUE(table.Test(1024));
   ASSERT_FALSE(table.Test(1025));
   ASSERT_FALSE(table.Test(2047));
   ASSERT_TRUE(table.Test(2048));
   ASSERT_TRUE(table.Test(2049));
   ASSERT_FALSE(table.Test(4095));
 }
}