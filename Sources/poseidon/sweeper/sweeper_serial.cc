#include "poseidon/sweeper/sweeper.h"
#include "poseidon/sweeper/sweeper_serial.h"

namespace poseidon {
 bool SerialSweeper::Visit(RawObject* raw){
   DLOG(INFO) << "visiting " << (*raw);
   if(raw->IsMarked() || !raw->IsOld())
     return true; // dont sweep marked objects
   return Sweeper::SweepObject(free_list(), raw);
 }
}