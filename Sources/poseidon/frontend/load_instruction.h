#ifndef POSEIDON_LOAD_INSTRUCTION_H
#define POSEIDON_LOAD_INSTRUCTION_H

#include "poseidon/frontend/instruction.h"

namespace poseidon {
 class LoadInstruction : public Instruction {
  public:
   LoadInstruction() = default;
   ~LoadInstruction() override = default;

   DECLARE_INSTRUCTION(LoadInstruction);
 };
}

#endif//POSEIDON_LOAD_INSTRUCTION_H