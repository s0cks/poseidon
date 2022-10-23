#ifndef POSEIDON_STORE_INSTRUCTION_H
#define POSEIDON_STORE_INSTRUCTION_H

#include "poseidon/frontend/instruction.h"

namespace poseidon {
 class StoreInstruction : public Instruction {
  public:
   StoreInstruction() = default;
   ~StoreInstruction() override = default;

   DECLARE_INSTRUCTION(StoreInstruction);
 };
}

#endif //POSEIDON_STORE_INSTRUCTION_H