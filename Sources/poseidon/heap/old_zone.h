#ifndef POSEIDON_HEAP_OLD_ZONE_H
#define POSEIDON_HEAP_OLD_ZONE_H

#include "poseidon/flags.h"
#include "poseidon/bitset.h"
#include "poseidon/heap/zone.h"
#include "poseidon/heap/old_page.h"
#include "poseidon/heap/freelist.h"

namespace poseidon{
 class OldZone : public Zone{
   friend class OldZoneTest;
   friend class SweeperTest;
   friend class Heap;
   friend class Scavenger;

   friend class SerialSweeper;
   friend class ParallelSweeper;
  protected:
   int64_t page_size_;
   FreeList* free_list_;

   uword TryAllocate(int64_t size) override;
  public:
   int64_t GetPageIndexFor(uword address){
     PSDN_ASSERT(Contains(address));
     int64_t index = static_cast<int64_t>(address - GetStartingAddress()) / page_size_;
     return index;
   }
  public:
   explicit OldZone(const MemoryRegion& region):
    Zone(region, GetOldPageSize()) {
     if(!region.Protect(MemoryRegion::kReadWrite)) {
       LOG(FATAL) << "failed to protect " << region;
     }
     InitializePageTable(region, CalculateNumberOfPages(region, GetOldPageSize()), GetOldPageSize());
   }
   ~OldZone() override = default;

   FreeList* free_list(){//TODO: visible for testing
     return free_list_;
   }

   int64_t GetAllocatableSize() const {
     return GetSize();
   }

   OldZone& operator=(const OldZone& rhs){
     if(this == &rhs)
       return *this;
     Zone::operator=(rhs);
     return *this;
   }

   friend std::ostream& operator<<(std::ostream& stream, const OldZone& val){
     return stream << (Zone&)val;
   }
 };
}

#endif//POSEIDON_HEAP_OLD_ZONE_H