#ifndef POSEIDON_GRAPH_ENTRY_INSTR_H
#define POSEIDON_GRAPH_ENTRY_INSTR_H

#include "poseidon/frontend/block_entry_instr.h"

namespace poseidon {
 class GraphEntryInstruction : public BlockEntryInstruction {
  public:
   GraphEntryInstruction() = default;
   ~GraphEntryInstruction() override = default;
   DECLARE_INSTRUCTION(GraphEntryInstruction);
 };
}

#endif //POSEIDON_GRAPH_ENTRY_INSTR_H