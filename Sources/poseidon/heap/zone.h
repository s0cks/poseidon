#ifndef POSEIDON_HEAP_ZONE_H
#define POSEIDON_HEAP_ZONE_H

#include "poseidon/raw_object.h"
#include "poseidon/heap/section.h"
#include "poseidon/heap/page_table.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon{
 class Zone : public AllocationSection {
   friend class ZoneTest;
   friend class RawObject;
   friend class Scavenger;
  protected:
   static inline int64_t
   CalculateNumberOfPages(const MemoryRegion& region, const int64_t page_size) { //TODO: cleanup
     return region.GetSize() / page_size;
   }
  protected:
   MemoryRegion region_;
   RelaxedAtomic<MemoryRegion::ProtectionMode> mode_; //TODO: remove
   PageTable pages_;

   inline MemoryRegion::ProtectionMode
   GetMode() const{
     return (MemoryRegion::ProtectionMode)mode_;
   }

   inline void
   SetMode(const MemoryRegion::ProtectionMode& mode){
     if(!region_.Protect(mode)){
       LOG(FATAL) << "failed to set " << (*this) << " to " << mode;
       return;
     }
     DLOG(INFO) << "set " << (*this) << " to " << mode;
     mode_ = mode;
   }

   inline void
   SetWriteable(){
     if(IsWritable()){
       DLOG(WARNING) << (*this) << " is already writeable.";
       return;
     }
     return SetMode(MemoryRegion::kReadWrite);
   }

   inline void
   SetReadOnly(){
     if(IsReadOnly()){
       DLOG(WARNING) << (*this) << " is already read-only.";
       return;
     }
     return SetMode(MemoryRegion::kReadOnly);
   }

   inline PageTable& pages() {
     return pages_;
   }

   virtual void InitializePageTable(const MemoryRegion& region, int64_t num_pages, int64_t page_size) {
     // do nothing, TODO: clean this up
   }

   inline void PutPage(Page* page) {
     pages_.pages_[page->index()] = page;
   }
  public:
   Zone():
    AllocationSection(),
    region_(),
    mode_(MemoryRegion::kReadOnly),
    pages_(0) {
     InitializePageTable(MemoryRegion(), 0, 0);
   }

   /**
    * Create a {@link Zone} with the specified starting address and size.
    *
    * @param start The starting address for the {@link Zone}
    * @param size The size of the {@link Zone}
    */
   Zone(const MemoryRegion& region, const int64_t page_size):
    AllocationSection(region.GetStartingAddress(), region.GetSize()),
    region_(region),
    mode_(MemoryRegion::kReadOnly),
    pages_(CalculateNumberOfPages(region, page_size)){
     InitializePageTable(region, CalculateNumberOfPages(region, page_size), page_size);
   }

   Zone(const Zone& rhs) = default;
   ~Zone() override = default;

   BitSet marked_set() { //TODO: remove
     return pages().table_;
   }

   bool IsReadOnly() const{
     return GetMode() == MemoryRegion::kReadOnly;
   }

   bool IsWritable() const{
     return GetMode() == MemoryRegion::kReadWrite;
   }

   inline void
   Mark(Page* page) {
     pages().Mark(page);
   }

   inline void
   Mark(const PageIndex index) {
     return Mark(pages().pages(index));
   }

   inline void
   MarkAll(const Region& region) {
     return pages().MarkAll(region);
   }

   inline void
   MarkAll(const RawObject* ptr) {
     return MarkAll((*ptr));
   }

   inline void
   Unmark(Page* page) {
     pages().Unmark(page);
   }

   inline void
   Unmark(const PageIndex index) {
     return Unmark(pages().pages(index));
   }

   virtual void VisitPages(PageVisitor* vis) const {
     return pages_.VisitPages(vis);
   }

   Zone& operator=(const Zone& rhs){
     if(*this == rhs)
       return *this;
     AllocationSection::operator=(rhs);
     region_ = rhs.region_;
     mode_ = rhs.GetMode();
     return *this;
   }

   friend std::ostream& operator<<(std::ostream& stream, const Zone& zone){
     stream << "Zone(";
     stream << "starting_address=" << zone.GetStartingAddressPointer() << ", ";
     stream << "total_size=" << Bytes(zone.GetSize());
     stream << ")";
     return stream;
   }
 };
}

#endif //POSEIDON_HEAP_ZONE_H
