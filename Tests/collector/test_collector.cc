#include <gtest/gtest.h>
#include <glog/logging.h>

#include "poseidon/local.h"
#include "poseidon/collector/collector.h"
#include "poseidon/allocator/allocator.h"

namespace poseidon{
 using namespace ::testing;

 class CollectorTest : public Test{
  protected:
   void SetUp() override{
     Heap::ResetCurrentThreadHeap();
     LocalPage::ResetLocalPageForCurrentThread();
   }
  public:
   CollectorTest() = default;
   ~CollectorTest() override = default;
 };

 static inline Local<word>
 AllocateRootWord(word value){
   auto raw_ptr = (RawObject*)Allocator::Allocate(sizeof(word));
   *((word*)raw_ptr->GetPointer()) = value;

   Local<word> handle;
   handle = raw_ptr->GetAddress();
   return handle;
 }

 static inline RawObject*
 AllocateWord(word value){
   auto raw_ptr = (RawObject*)Allocator::Allocate(sizeof(word));
   *((word*)raw_ptr->GetPointer()) = value;
   return raw_ptr;
 }

 TEST_F(CollectorTest, TestMinorCollection){
   // r1 is a root object that will persist all collections
   static const constexpr int64_t kR1DefaultValue = 100;
   auto r1 = AllocateRootWord(kR1DefaultValue);
   ASSERT_EQ(*r1.Get(), kR1DefaultValue);
   ASSERT_TRUE(r1.raw()->IsNew());
   ASSERT_FALSE(r1.raw()->IsOld());
   ASSERT_FALSE(r1.raw()->IsMarked());
   ASSERT_FALSE(r1.raw()->IsRemembered());
   ASSERT_FALSE(r1.raw()->IsForwarding());

   // r2 is a root object that will persist all collections
   static const constexpr int64_t kR2DefaultValue = 1000;
   auto r2 = AllocateRootWord(kR2DefaultValue);
   ASSERT_EQ(*r2.Get(), kR2DefaultValue);
   ASSERT_TRUE(r2.raw()->IsNew());
   ASSERT_FALSE(r2.raw()->IsOld());
   ASSERT_FALSE(r2.raw()->IsMarked());
   ASSERT_FALSE(r2.raw()->IsRemembered());
   ASSERT_FALSE(r2.raw()->IsForwarding());

   // r3 is a root object that will persist all collections
   static const constexpr int64_t kR3DefaultValue = 10;
   auto r3 = AllocateRootWord(kR3DefaultValue);
   ASSERT_EQ(*r3.Get(), kR3DefaultValue);
   ASSERT_TRUE(r3.raw()->IsNew());
   ASSERT_FALSE(r3.raw()->IsOld());
   ASSERT_FALSE(r3.raw()->IsMarked());
   ASSERT_FALSE(r3.raw()->IsRemembered());
   ASSERT_FALSE(r3.raw()->IsForwarding());

   // g1 is a garbage object that gets collected on the first MinorCollection
   static const constexpr int64_t kG1DefaultValue = 10;
   auto g1 = AllocateWord(kG1DefaultValue);
   ASSERT_EQ(*((word*)g1->GetPointer()), kG1DefaultValue);
   ASSERT_TRUE(g1->IsNew());
   ASSERT_FALSE(g1->IsOld());
   ASSERT_FALSE(g1->IsMarked());
   ASSERT_FALSE(g1->IsRemembered());
   ASSERT_FALSE(g1->IsForwarding());

   // g2 is a garbage object that gets collected on the first MinorCollection
   static const constexpr int64_t kG2DefaultValue = 1000000;
   auto g2 = AllocateWord(kG2DefaultValue);
   ASSERT_EQ(*((word*)g2->GetPointer()), kG2DefaultValue);
   ASSERT_TRUE(g2->IsNew());
   ASSERT_FALSE(g2->IsOld());
   ASSERT_FALSE(g2->IsMarked());
   ASSERT_FALSE(g2->IsRemembered());
   ASSERT_FALSE(g2->IsForwarding());

   LOG(INFO) << "r1 (before): " << (*r1.Get()) << " (" << r1.raw()->ToString() << ").";
   LOG(INFO) << "r2 (before): " << (*r2.Get()) << " (" << r2.raw()->ToString() << ").";
   LOG(INFO) << "r3 (before): " << (*r3.Get()) << " (" << r3.raw()->ToString() << ").";
   LOG(INFO) << "g1 (before): " << (*((word*)g1->GetPointer())) << " (" << g1->ToString() << ").";
   LOG(INFO) << "g2 (before): " << (*((word*)g2->GetPointer())) << " (" << g2->ToString() << ").";

   Collector::MinorCollection();

   LOG(INFO) << "r1 (after): " << (*r1.Get()) << " (" << r1.raw()->ToString() << ").";
   LOG(INFO) << "r2 (after): " << (*r2.Get()) << " (" << r2.raw()->ToString() << ").";
   LOG(INFO) << "r3 (after): " << (*r3.Get()) << " (" << r3.raw()->ToString() << ").";
   LOG(INFO) << "g1 (after): " << (*((word*)g1->GetPointer())) << " (" << g1->ToString() << ").";
   LOG(INFO) << "g2 (after): " << (*((word*)g2->GetPointer())) << " (" << g2->ToString() << ").";

   // check that r1 has persisted the first MinorCollection
   ASSERT_EQ(*r1.Get(), kR1DefaultValue);
   ASSERT_TRUE(r1.raw()->IsNew());
   ASSERT_FALSE(r1.raw()->IsOld());
   ASSERT_FALSE(r1.raw()->IsMarked());
   ASSERT_TRUE(r1.raw()->IsRemembered()); // r1 should be marked as remembered now
   ASSERT_FALSE(r1.raw()->IsForwarding());

   // check that r2 has persisted the first MinorCollection
   ASSERT_EQ(*r2.Get(), kR2DefaultValue);
   ASSERT_TRUE(r2.raw()->IsNew());
   ASSERT_FALSE(r2.raw()->IsOld());
   ASSERT_FALSE(r2.raw()->IsMarked());
   ASSERT_TRUE(r2.raw()->IsRemembered()); // r1 should be marked as remembered now
   ASSERT_FALSE(r2.raw()->IsForwarding());

   // check that r3 has persisted the first MinorCollection
   ASSERT_EQ(*r3.Get(), kR3DefaultValue);
   ASSERT_TRUE(r3.raw()->IsNew());
   ASSERT_FALSE(r3.raw()->IsOld());
   ASSERT_FALSE(r3.raw()->IsMarked());
   ASSERT_TRUE(r3.raw()->IsRemembered()); // r3 should be marked as remembered now
   ASSERT_FALSE(r3.raw()->IsForwarding());
 }

 TEST_F(CollectorTest, TestMinorCollectionWithPromotions){
   // r1 is a root object that will persist all collections
   static const constexpr int64_t kR1DefaultValue = 100;
   auto r1 = AllocateRootWord(kR1DefaultValue);
   ASSERT_EQ(*r1.Get(), kR1DefaultValue);
   ASSERT_TRUE(r1.raw()->IsNew());
   ASSERT_FALSE(r1.raw()->IsOld());
   ASSERT_FALSE(r1.raw()->IsMarked());
   ASSERT_FALSE(r1.raw()->IsRemembered());
   ASSERT_FALSE(r1.raw()->IsForwarding());

   // r2 is a root object that will persist all collections
   static const constexpr int64_t kR2DefaultValue = 1000;
   auto r2 = AllocateRootWord(kR2DefaultValue);
   ASSERT_EQ(*r2.Get(), kR2DefaultValue);
   ASSERT_TRUE(r2.raw()->IsNew());
   ASSERT_FALSE(r2.raw()->IsOld());
   ASSERT_FALSE(r2.raw()->IsMarked());
   ASSERT_FALSE(r2.raw()->IsRemembered());
   ASSERT_FALSE(r2.raw()->IsForwarding());

   // r3 is a root object that will persist all collections
   static const constexpr int64_t kR3DefaultValue = 10;
   auto r3 = AllocateRootWord(kR3DefaultValue);
   ASSERT_EQ(*r3.Get(), kR3DefaultValue);
   ASSERT_TRUE(r3.raw()->IsNew());
   ASSERT_FALSE(r3.raw()->IsOld());
   ASSERT_FALSE(r3.raw()->IsMarked());
   ASSERT_FALSE(r3.raw()->IsRemembered());
   ASSERT_FALSE(r3.raw()->IsForwarding());

   // g1 is a garbage object that gets collected on the first MinorCollection
   static const constexpr int64_t kG1DefaultValue = 10;
   auto g1 = AllocateWord(kG1DefaultValue);
   ASSERT_EQ(*((word*)g1->GetPointer()), kG1DefaultValue);
   ASSERT_TRUE(g1->IsNew());
   ASSERT_FALSE(g1->IsOld());
   ASSERT_FALSE(g1->IsMarked());
   ASSERT_FALSE(g1->IsRemembered());
   ASSERT_FALSE(g1->IsForwarding());

   // g2 is a garbage object that gets collected on the first MinorCollection
   static const constexpr int64_t kG2DefaultValue = 1000000;
   auto g2 = AllocateWord(kG2DefaultValue);
   ASSERT_EQ(*((word*)g2->GetPointer()), kG2DefaultValue);
   ASSERT_TRUE(g2->IsNew());
   ASSERT_FALSE(g2->IsOld());
   ASSERT_FALSE(g2->IsMarked());
   ASSERT_FALSE(g2->IsRemembered());
   ASSERT_FALSE(g2->IsForwarding());

   LOG(INFO) << "r1 (before): " << (*r1.Get()) << " (" << r1.raw()->ToString() << ").";
   LOG(INFO) << "r2 (before): " << (*r2.Get()) << " (" << r2.raw()->ToString() << ").";
   LOG(INFO) << "r3 (before): " << (*r3.Get()) << " (" << r3.raw()->ToString() << ").";
   LOG(INFO) << "g1 (before): " << (*((word*)g1->GetPointer())) << " (" << g1->ToString() << ").";
   LOG(INFO) << "g2 (before): " << (*((word*)g2->GetPointer())) << " (" << g2->ToString() << ").";
   Collector::MinorCollection();

   LOG(INFO) << "r1 (inter): " << (*r1.Get()) << " (" << r1.raw()->ToString() << ").";
   LOG(INFO) << "r2 (inter): " << (*r2.Get()) << " (" << r2.raw()->ToString() << ").";
   LOG(INFO) << "r3 (inter): " << (*r3.Get()) << " (" << r3.raw()->ToString() << ").";
   LOG(INFO) << "g1 (inter): " << (*((word*)g1->GetPointer())) << " (" << g1->ToString() << ").";
   LOG(INFO) << "g2 (inter): " << (*((word*)g2->GetPointer())) << " (" << g2->ToString() << ").";

   // check that r1 has persisted the first MinorCollection
   ASSERT_EQ(*r1.Get(), kR1DefaultValue);
   ASSERT_TRUE(r1.raw()->IsNew());
   ASSERT_FALSE(r1.raw()->IsOld());
   ASSERT_FALSE(r1.raw()->IsMarked());
   ASSERT_TRUE(r1.raw()->IsRemembered()); // r1 should be marked as remembered now
   ASSERT_FALSE(r1.raw()->IsForwarding());

   // check that r2 has persisted the first MinorCollection
   ASSERT_EQ(*r2.Get(), kR2DefaultValue);
   ASSERT_TRUE(r2.raw()->IsNew());
   ASSERT_FALSE(r2.raw()->IsOld());
   ASSERT_FALSE(r2.raw()->IsMarked());
   ASSERT_TRUE(r2.raw()->IsRemembered()); // r1 should be marked as remembered now
   ASSERT_FALSE(r2.raw()->IsForwarding());

   // check that r3 has persisted the first MinorCollection
   ASSERT_EQ(*r3.Get(), kR3DefaultValue);
   ASSERT_TRUE(r3.raw()->IsNew());
   ASSERT_FALSE(r3.raw()->IsOld());
   ASSERT_FALSE(r3.raw()->IsMarked());
   ASSERT_TRUE(r3.raw()->IsRemembered()); // r3 should be marked as remembered now
   ASSERT_FALSE(r3.raw()->IsForwarding());

   Collector::MinorCollection();
   LOG(INFO) << "r1 (after): " << (*r1.Get()) << " (" << r1.raw()->ToString() << ").";
   LOG(INFO) << "r2 (after): " << (*r2.Get()) << " (" << r2.raw()->ToString() << ").";
   LOG(INFO) << "r3 (after): " << (*r3.Get()) << " (" << r3.raw()->ToString() << ").";
   LOG(INFO) << "g1 (after): " << (*((word*)g1->GetPointer())) << " (" << g1->ToString() << ").";
   LOG(INFO) << "g2 (after): " << (*((word*)g2->GetPointer())) << " (" << g2->ToString() << ").";

   // check that r1 has persisted the second MinorCollection
   ASSERT_EQ(*r1.Get(), kR1DefaultValue);
   ASSERT_FALSE(r1.raw()->IsNew());
   ASSERT_TRUE(r1.raw()->IsOld()); // r1 should now be marked as old
   ASSERT_FALSE(r1.raw()->IsMarked());
   ASSERT_TRUE(r1.raw()->IsRemembered()); // r1 should not be marked as remembered anymore
   ASSERT_FALSE(r1.raw()->IsForwarding());

   // check that r2 has persisted the second MinorCollection
   ASSERT_EQ(*r2.Get(), kR2DefaultValue);
   ASSERT_FALSE(r2.raw()->IsNew());
   ASSERT_TRUE(r2.raw()->IsOld()); // r2 should now be marked as old
   ASSERT_FALSE(r2.raw()->IsMarked());
   ASSERT_TRUE(r2.raw()->IsRemembered()); // r2 should not be marked as remembered anymore
   ASSERT_FALSE(r2.raw()->IsForwarding());

   // check that r3 has persisted the second MinorCollection
   ASSERT_EQ(*r3.Get(), kR3DefaultValue);
   ASSERT_FALSE(r3.raw()->IsNew());
   ASSERT_TRUE(r3.raw()->IsOld()); // r3 should now be marked as old
   ASSERT_FALSE(r3.raw()->IsMarked());
   ASSERT_TRUE(r3.raw()->IsRemembered()); // r3 should not be marked as remembered anymore
   ASSERT_FALSE(r3.raw()->IsForwarding());
 }

 TEST_F(CollectorTest, TestMajorCollection){

 }
}