#ifndef POSEIDON_TEST_INSTRUCTION_H
#define POSEIDON_TEST_INSTRUCTION_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "poseidon/frontend/instruction.h"
#include "poseidon/frontend/load_instruction.h"
#include "poseidon/frontend/graph_entry_instr.h"
#include "poseidon/frontend/store_instruction.h"

namespace poseidon {
 using namespace ::testing;

 class InstructionTest : public Test {
  protected:
   InstructionTest() = default;
  public:
   ~InstructionTest() override = default;
 };
}

#endif // POSEIDON_TEST_INSTRUCTION_H