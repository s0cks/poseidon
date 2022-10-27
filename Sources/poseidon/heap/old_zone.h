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
  public:
   class OldZoneIterator : public ZoneIterator<OldZone> {
    public:
     explicit OldZoneIterator(OldZone* zone):
       ZoneIterator<OldZone>(zone) {
     }
     ~OldZoneIterator() override = default;

     bool HasNext() const override {
       return current_address() > 0 &&
              current_address() < zone()->GetEndingAddress() &&
              current_ptr()->IsOld() &&
              current_ptr()->GetSize() > 0;
     }
   };

   class OldZonePageIterator {
    protected:
     OldZone* zone_;
     PageIndex current_;

     inline OldZone* zone() const {
       return zone_;
     }

     inline PageIndex current_index() const {
       return current_;
     }

     inline OldPage* current_page() const {
       return zone()->pages(current_index());
     }
    public:
     explicit OldZonePageIterator(OldZone* zone):
      zone_(zone),
      current_(0) {
     }
     ~OldZonePageIterator() = default;

     bool HasNext() const {
       return current_index() >= 0 &&
           current_index() < zone()->GetNumberOfPages();
     }

     OldPage* Next() {
       auto next = current_page();
       current_ += 1;
       return next;
     }
   };
  protected:
   FreeList* free_list_; //TODO: change to value
   OldPage* pages_;
   PageIndex num_pages_;
   BitSet table_;

   bool InitializePages(const MemoryRegion& region);
   uword TryAllocate(int64_t size);

   inline bool MarkAllIntersectedBy(const Region& region) {
     for(auto it = pages_begin(); it != pages_end(); it++) {
       if(it->Contains(region) && !Mark(it->index()))
         return false;
     }
     return true;
   }
  public:
   OldZone():
    Zone(),
    free_list_(new FreeList()),
    pages_(nullptr),
    num_pages_(0),
    table_() {
   }
   explicit OldZone(const MemoryRegion& region):
    Zone(region),
    free_list_(new FreeList(region)),
    pages_(nullptr),
    num_pages_(0),
    table_() {
     LOG_IF(FATAL, !region.Protect(MemoryRegion::kReadWrite)) << "failed to protect " << region;
     LOG_IF(FATAL, !InitializePages(region)) << "failed to initialize pages for " << (*this);
   }
   OldZone(const OldZone& rhs) = default;
   ~OldZone() override = default;

   FreeList* free_list(){//TODO: visible for testing
     return free_list_;
   }

   OldPage* pages() const {
     return pages_;
   }

   OldPage* pages(const PageIndex& index) const {
     if(index < 0 || index >= num_pages_)
       return nullptr;
     return pages() + index;
   }

   OldPage* pages_begin() const {
     return pages();
   }

   OldPage* pages_end() const {
     return pages() + GetNumberOfPages();
   }

   PageIndex GetNumberOfPages() const {
     return num_pages_;
   }

   bool IsMarked(const PageIndex& index) const {
     auto page = pages(index);
     if(page == nullptr)
       return false;
     return table_.Test(index) && page->marked();
   }

   bool Mark(const PageIndex& index) {
     auto page = pages(index);
     if(page == nullptr)
       return false;
     DLOG(INFO) << "marking " << (*page);
     table_.Set(index);
     page->SetMarkedBit();
     return IsMarked(index);
   }

   bool VisitPages(PageVisitor* vis) override;
   bool VisitMarkedPages(PageVisitor* vis) override;
   bool VisitUnmarkedPages(PageVisitor* vis) override;
   bool VisitPointers(RawObjectVisitor* vis) override;
   bool VisitMarkedPointers(RawObjectVisitor* vis) override;

   OldZone& operator=(const OldZone& rhs) = default;

   friend std::ostream& operator<<(std::ostream& stream, const OldZone& val){
     return stream << (Zone&)val;
   }
 };
}

#endif//POSEIDON_HEAP_OLD_ZONE_H