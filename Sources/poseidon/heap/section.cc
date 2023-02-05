#include "poseidon/pointer.h"
#include "poseidon/heap/section.h"

namespace poseidon{
 void Section::Protect(MemoryRegion::ProtectionMode mode) {
   LOG_IF(FATAL, !region().Protect(mode)) << "cannot protect " << (*this);
 }
}