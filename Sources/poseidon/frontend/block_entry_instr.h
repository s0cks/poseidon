#ifndef POSEIDON_BLOCK_ENTRY_INSTR_H
#define POSEIDON_BLOCK_ENTRY_INSTR_H

#include "poseidon/frontend/instruction.h"

namespace poseidon {
 class BlockEntryInstruction : public Instruction {
  public:
   BlockEntryInstruction() = default;
   ~BlockEntryInstruction() override = default;
 };
}

#endif // POSEIDON_BLOCK_ENTRY_INSTR_H