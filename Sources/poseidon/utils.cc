#include "poseidon/utils.h"
#include "poseidon/raw_object.h"

namespace poseidon{
 AtomicPointerCounter& AtomicPointerCounter::operator+=(RawObject* val){
   count += 1;
   bytes += val->GetTotalSize();
   return *this;
 }

 AtomicPointerCounter& AtomicPointerCounter::operator-=(RawObject* val){
   count -= 1;
   bytes -= val->GetTotalSize();
   return *this;
 }
}