#ifndef POSEIDON_OLD_ZONE_H
#define POSEIDON_OLD_ZONE_H

#include "poseidon/bitset.h"
#include "poseidon/freelist.h"
#include "poseidon/heap/zone.h"

#include "poseidon/heap/old_page.h"

namespace poseidon{
 class OldZone : public Zone{
   friend class OldZoneTest;
   friend class SweeperTest;

   friend class SerialSweeper;
   friend class ParallelSweeper;
  protected:
   FreeList* free_list_;
   OldPageTable pages_;

   static inline int64_t
   CalculateTableSize(int64_t size, int64_t page_size){
     return size / page_size;
   }

   OldZone(uword start, int64_t size, int64_t page_size, FreeList* free_list):// visible for testing?
    Zone(start, size),
    pages_(start, size, page_size),
    free_list_(free_list){
     SetWriteable();
   }

   OldZone(MemoryRegion* region, int64_t offset, int64_t size, int64_t page_size, FreeList* free_list):
    OldZone(region->GetStartingAddress() + offset, size, page_size, free_list){
   }

   OldZone(MemoryRegion* region, int64_t size, int64_t page_size, FreeList* free_list):
    OldZone(region, 0, size, page_size, free_list){
   }

   OldZone(MemoryRegion* region, int64_t page_size, FreeList* free_list):
    OldZone(region, region->size(), page_size, free_list){
   }
  public:
   OldZone(uword start, int64_t size, int64_t page_size):
     Zone(start, size),
     pages_(start, size, page_size),
     free_list_(new FreeList(start, size)){
     SetWriteable();
   }

   OldZone(MemoryRegion* region, int64_t offset, int64_t size, int64_t page_size):
     OldZone(region->GetStartingAddress() + offset, size, page_size){
   }

   OldZone(MemoryRegion* region, int64_t size, int64_t page_size):
    OldZone(region, 0, size, page_size){
   }

   OldZone(MemoryRegion* region, int64_t page_size):
    OldZone(region, region->size(), page_size){
   }
   ~OldZone() override = default;

   FreeList* free_list(){//TODO: visible for testing
     return free_list_;
   }

   uword TryAllocate(int64_t size) override;
   void VisitPages(const std::function<bool(OldPage*)>& vis) const;
   void VisitMarkedPages(const std::function<bool(OldPage*)>& vis) const;

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

#endif//POSEIDON_OLD_ZONE_H