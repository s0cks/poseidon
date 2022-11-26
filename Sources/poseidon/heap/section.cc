#include "poseidon/pointer.h"
#include "poseidon/heap/section.h"

namespace poseidon{
 void Section::Protect(MemoryRegion::ProtectionMode mode) {
   MemoryRegion region(GetStartingAddress(), GetSize());
   LOG_IF(FATAL, !region.Protect(mode)) << "cannot protect " << (*this);
 }
}