#ifndef POSEIDON_HEAP_OLD_ZONE_H
#define POSEIDON_HEAP_OLD_ZONE_H

#include "poseidon/flags.h"
#include "poseidon/bitset.h"
#include "poseidon/heap/zone.h"
#include "poseidon/heap/old_page.h"
#include "poseidon/heap/freelist.h"

namespace poseidon{
 class OldZone : public Zone {
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

   class OldZonePageIterator : public ZonePageIterator<OldZone, OldPage> {
    public:
     explicit OldZonePageIterator(OldZone* zone):
       ZonePageIterator<OldZone, OldPage>(zone, GetNumberOfOldPages()) {
     }
     ~OldZonePageIterator() override = default;
   };

   static inline ObjectSize
   GetHeaderSize() {
     return sizeof(OldZone);
   }
  protected:
   BitSet table_;
   FreeList free_list_;

   OldZone(const uword start_address, const int64_t size, const int64_t num_pages):
    Zone(),
    table_(num_pages),
    free_list_(start_address, size) {
   }

   inline uword GetPageAddressAt(const int64_t index) const {
     if(index < 0 || index > GetNumberOfOldPages())
       return 0;
     return GetStartingAddress() + (index * GetNewPageSize());
   }

   inline int64_t GetPageIndex(Page* page) const {
     return static_cast<int64_t>((page->GetStartingAddress() - GetStartingAddress())) / GetNewPageSize();
   }

   virtual bool MarkAllIntersectedBy(const Region& region);
  public:
   OldZone() = delete;
   ~OldZone() override = default; //TODO: change to delete

   uword GetStartingAddress() const override {
     return GetZoneStartingAddress() + GetHeaderSize();
   }

   uword GetZoneStartingAddress() const {
     return (uword)this;
   }

   int64_t GetSize() const override {
     return GetOldZoneSize();
   }

   int64_t GetTotalSize() const {
     return GetHeaderSize() + GetSize();
   }

   FreeList* free_list(){//TODO: visible for testing
     NOT_IMPLEMENTED(FATAL); //TODO: implement
     return nullptr;
   }

   virtual OldPage* GetPageAt(const int64_t index) const {
     if(index < 0 || index > GetNumberOfOldPages())
       return nullptr;
     return OldPage::From(GetPageAddressAt(index));
   }

   virtual bool IsPageMarked(const int64_t index) const {
     return table_.Test(index);
   }

   inline bool IsPageMarked(OldPage* page) const {
     return IsPageMarked(GetPageIndex(page));
   }

   virtual inline bool MarkPage(const int64_t index) {
     table_.Set(index, true);
     return IsPageMarked(index);
   }

   virtual inline bool MarkPage(Page* page) {
     return MarkPage(GetPageIndex(page));
   }

   virtual inline bool UnmarkPage(const int64_t index) {
     table_.Set(index, false);
     return !IsPageMarked(index);
   }

   virtual inline bool UnmarkPage(Page* page) {
     return UnmarkPage(GetPageIndex(page));
   }

   uword TryAllocate(const ObjectSize& size);

   virtual bool VisitPages(PageVisitor* vis) {
     return IteratePages<OldZone, OldZonePageIterator, PageVisitor>(this, vis);
   }

   virtual bool VisitPages(OldPageVisitor* vis) {
     return IteratePages<OldZone, OldZonePageIterator, OldPageVisitor>(this, vis);
   }

   virtual bool VisitMarkedPages(PageVisitor* vis) {
     return IteratePages<OldZone, OldZonePageIterator, PageVisitor>(this, vis);
   }

   virtual bool VisitMarkedPages(OldPageVisitor* vis) {
     return IterateMarkedPages<OldZone, OldZonePageIterator, OldPageVisitor>(this, vis);
   }

   bool VisitPointers(RawObjectVisitor* vis) override {
     return IteratePointers<OldZone, OldZoneIterator, RawObjectVisitor>(this, vis);
   }

   bool VisitMarkedPointers(RawObjectVisitor* vis) override {
     return IterateMarkedPointers<OldZone, OldZoneIterator, RawObjectVisitor>(this, vis);
   }

   friend std::ostream& operator<<(std::ostream& stream, const OldZone& val){
     stream << "OldZone(";
     stream << "start=" << val.GetStartingAddressPointer() << ", ";
     stream << "size=" << Bytes(val.GetSize());
     stream << ")";
     return stream;
   }

   static OldZone* From(const MemoryRegion& region);
 };
}

#endif//POSEIDON_HEAP_OLD_ZONE_H