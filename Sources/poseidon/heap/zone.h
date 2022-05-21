#ifndef POSEIDON_POSEIDON_ZONE_H
#define POSEIDON_POSEIDON_ZONE_H

#include "poseidon/raw_object.h"
#include "poseidon/heap/section.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon{
 class Zone : public AllocationSection{
   friend class ZoneTest;
   friend class RawObject;
   friend class Scavenger;
  private:
  public:
   Zone() = default;

   /**
    * Create a {@link Zone} with the specified starting address and size.
    *
    * @param start The starting address for the {@link Zone}
    * @param size The size of the {@link Zone}
    */
   Zone(uword start, int64_t size)://TODO: cleanup?
    AllocationSection(start, size){
   }

   Zone(MemoryRegion* region, int64_t offset, int64_t size):
    Zone(region->GetStartingAddress() + offset, size){
   }

   Zone(MemoryRegion* region, int64_t size):
    Zone(region, 0, size){
   }

   explicit Zone(MemoryRegion* region):
    Zone(region, region->size()){
   }

   Zone(const Zone& rhs) = default;
   ~Zone() override = default;

   Zone& operator=(const Zone& rhs) = default;

   friend std::ostream& operator<<(std::ostream& stream, const Zone& zone){
     stream << "Zone(";
     stream << "starting_address=" << zone.GetStartingAddressPointer() << ", ";
     stream << "total_size=" << Bytes(zone.GetSize());
     stream << ")";
     return stream;
   }
 };
}

#endif //POSEIDON_POSEIDON_ZONE_H
