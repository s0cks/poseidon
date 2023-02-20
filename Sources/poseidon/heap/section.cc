#include "poseidon/pointer.h"
#include "poseidon/heap/section.h"

namespace poseidon{
 void Section::Protect(MemoryRegion::ProtectionMode mode) {
   MemoryRegion memory_region(*this);
   LOG_IF(FATAL, !memory_region.Protect(mode)) << "cannot protect " << (*this);
 }
}