#include "poseidon/utils.h"
#include "poseidon/pointer.h"

namespace poseidon{
 AtomicPointerCounter& AtomicPointerCounter::operator+=(Pointer* val){
   count += 1;
   bytes += val->GetTotalSize();
   return *this;
 }

 AtomicPointerCounter& AtomicPointerCounter::operator-=(Pointer* val){
   count -= 1;
   bytes -= val->GetTotalSize();
   return *this;
 }
}