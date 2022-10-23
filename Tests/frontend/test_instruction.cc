#include "frontend/test_instruction.h"

namespace poseidon {
 TEST_F(InstructionTest, TestAppend) {
   GraphEntryInstruction g1;
   LoadInstruction l1;
   StoreInstruction s1;

   LoadInstruction l2;
   StoreInstruction s2;

   LoadInstruction l3;
   StoreInstruction s3;

   Instruction* list = &g1;
   Instruction::Append(&list, &l1);
   ASSERT_TRUE(g1.HasNext());
   ASSERT_EQ(g1.GetNext(), &l1);

   Instruction::Append(&list, &s1);
   ASSERT_TRUE(l1.HasNext());
   ASSERT_EQ(l1.GetNext(), &s1);

   Instruction::Append(&list, &l2);
   ASSERT_TRUE(s1.HasNext());
   ASSERT_EQ(s1.GetNext(), &l2);

   Instruction::Append(&list, &s2);
   ASSERT_TRUE(l2.HasNext());
   ASSERT_EQ(l2.GetNext(), &s2);
 }

 TEST_F(InstructionTest, TestPrepend) {

 }

 TEST_F(InstructionTest, TestInsertAfter) {

 }

 TEST_F(InstructionTest, TestInsertBefore) {

 }
}