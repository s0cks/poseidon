#ifndef POSEIDON_HEAP_NEW_ZONE_H
#define POSEIDON_HEAP_NEW_ZONE_H

#include "poseidon/flags.h"
#include "poseidon/heap/zone.h"
#include "poseidon/heap/new_page.h"
#include "poseidon/heap/semispace.h"
#include "poseidon/heap/page_table.h"

namespace poseidon{
 class NewZone : public Zone {//TODO: pages?
   template<bool Parallel>
   friend class ScavengerVisitorBase;//TODO: remove

   friend class Heap;
   friend class NewZoneTest;
  public:
   class NewZoneIterator : public ZoneIterator<NewZone> {
    public:
     explicit NewZoneIterator(NewZone* zone):
       ZoneIterator<NewZone>(zone) {
     }
     ~NewZoneIterator() override = default;

     bool HasNext() const override {
       return current_address() > 0 &&
              current_address() < zone()->GetEndingAddress() &&
              current_ptr()->IsNew() &&
              current_ptr()->GetSize() > 0;
     }
   };

   class NewZonePageIterator {
    protected:
     NewZone* zone_;
     PageIndex current_;

     inline NewZone* zone() const {
       return zone_;
     }

     inline PageIndex current_index() const {
       return current_;
     }

     inline NewPage* current_page() const {
       return zone()->pages(current_index());
     }
    public:
     explicit NewZonePageIterator(NewZone* zone):
      zone_(zone),
      current_(0) {
     }
     ~NewZonePageIterator() = default;

     bool HasNext() const {
       return current_index() >= 0 &&
              current_index() < zone()->GetNumberOfPages();
     }

     NewPage* Next() {
       auto next = current_page();
       current_ += 1;
       return next;
     }
   };
  protected:
   static inline uword
   GetFromspaceAddress(const MemoryRegion& region) {
     return region.GetStartingAddress();
   }

   static inline uword
   GetTospaceAddress(const MemoryRegion& region) {
     return region.GetStartingAddress() + GetNewZoneSemispaceSize();
   }
  protected:
   uword current_;
   uword fromspace_;
   uword tospace_;
   int64_t semisize_;
   NewPage* pages_;
   PageIndex num_pages_;
   BitSet table_;

   virtual void SwapSpaces(){
     std::swap(fromspace_, tospace_);
     current_ = tospace_;
   }

   bool InitializePages(const MemoryRegion& region);
   uword TryAllocate(int64_t size);

   inline MemoryRegion
   GetFromspaceRegion() const {
     return {fromspace(), semisize()};
   }

   inline Semispace
   GetFromspace() const {
     return Semispace(GetFromspaceRegion());
   }

   inline MemoryRegion
   GetTospaceRegion() const {
     return {tospace(), semisize()};
   }

   inline Semispace
   GetTospace() const {
     return Semispace(GetTospaceRegion());
   }

   inline void*
   tospace_ptr() const {
     return (void*)tospace();
   }

   inline void*
   fromspace_ptr() const {
     return (void*)fromspace();
   }

   inline bool MarkAllIntersectedBy(const Region& region) {
     for(auto it = pages_begin(); it != pages_end(); it++) {
       if(it->Contains(region) && !Mark(it->index()))
         return false;
     }
     return true;
   }
  public:
   NewZone():
     Zone(),
     current_(0),
     fromspace_(0),
     tospace_(0),
     semisize_(0),
     pages_(nullptr),
     num_pages_(0),
     table_() {
   }
   explicit NewZone(const MemoryRegion& region):
    Zone(region),
    current_(region.GetStartingAddress()),
    fromspace_(GetFromspaceAddress(region)),
    tospace_(GetTospaceAddress(region)),
    semisize_(GetNewZoneSemispaceSize()),
    pages_(nullptr),
    num_pages_(0),
    table_() {
     LOG_IF(FATAL, !region.Protect(MemoryRegion::kReadWrite)) << "failed to protect " << region;
     LOG_IF(FATAL, !InitializePages(region)) << "failed to initialize pages for " << (*this);
   }
   NewZone(const NewZone& rhs) = default;
   ~NewZone() override = default;

   uword GetCurrentAddress() const {
     return current_;
   }

   void* GetCurrentAddressPointer() const {
     return (void*)GetCurrentAddress();
   }

   uword tospace() const{
     return tospace_;
   }

   uword fromspace() const{
     return fromspace_;
   }

   int64_t semisize() const{
     return semisize_;
   }

   PageIndex GetNumberOfPages() const {
     return num_pages_;
   }

   NewPage* pages() const {
     return pages_;
   }

   NewPage* pages(const PageIndex& index) const {
     if(index < 0 || index >= num_pages_)
       return nullptr;
     return pages() + index;
   }

   NewPage* pages_begin() const {
     return pages();
   }

   NewPage* pages_end() const {
     return pages() + GetNumberOfPages();
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
     table_.Set(index);
     page->SetMarkedBit();
     return IsMarked(index);
   }

   bool VisitPages(PageVisitor* vis) override;
   bool VisitMarkedPages(PageVisitor* vis) override;
   bool VisitUnmarkedPages(PageVisitor* vis) override;
   bool VisitPointers(RawObjectVisitor* vis) override;
   bool VisitMarkedPointers(RawObjectVisitor* vis) override;

   NewZone& operator=(const NewZone& rhs) = default;

   friend std::ostream& operator<<(std::ostream& stream, const NewZone& val){
     stream << "NewZone(";
     stream << "start=" << val.GetStartingAddressPointer() << ", ";
     stream << "size=" << Bytes(val.GetSize()) << ", ";
     stream << "fromspace=" << val.fromspace_ptr() << ", ";
     stream << "tospace=" << val.tospace_ptr() << ", ";
     stream << "semi-size=" << Bytes(val.semisize());
     stream << ")";
     return stream;
   }

   friend bool operator==(const NewZone& lhs, const NewZone& rhs) {
     return ((const Zone&)lhs) == ((const Zone&)rhs) &&
            lhs.GetFromspace() == rhs.GetFromspace() &&
            lhs.GetTospace() == rhs.GetTospace();
   }

   friend bool operator!=(const NewZone& lhs, const NewZone& rhs) {
     return !operator==(lhs, rhs);
   }
 };
}

#endif//POSEIDON_HEAP_NEW_ZONE_H