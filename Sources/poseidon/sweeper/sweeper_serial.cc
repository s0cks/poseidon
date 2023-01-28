#include "poseidon/sweeper/sweeper.h"
#include "poseidon/sweeper/sweeper_serial.h"

namespace poseidon {
 bool SerialSweeper::Visit(Pointer* raw){
   if(raw->IsMarked() || !raw->IsOld() || raw->IsFree())
     return true; // dont sweep marked objects
   return sweeper()->Sweep(raw);
 }
}