#ifndef POSEIDON_HEAP_NEW_ZONE_H
#define POSEIDON_HEAP_NEW_ZONE_H

#include "poseidon/flags.h"
#include "poseidon/bitset.h"
#include "poseidon/heap/zone.h"
#include "poseidon/heap/new_page.h"
#include "poseidon/heap/semispace.h"

namespace poseidon{
 class NewZone : public Zone { //TODO: make resizable
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

   class NewZonePageIterator : public ZonePageIterator<NewZone, NewPage> {
    public:
     explicit NewZonePageIterator(NewZone* zone):
       ZonePageIterator<NewZone, NewPage>(zone, GetNumberOfNewPages()) {
     }
     ~NewZonePageIterator() override = default;
   };

   static inline constexpr int64_t
   GetHeaderSize() {
     return sizeof(NewZone);
   }
  protected:
   int64_t size_; //TODO: remove
   int64_t semisize_; //TODO: remove
   BitSet table_;
   Semispace fromspace_;
   Semispace tospace_;

   NewZone(const int64_t& size, const int64_t& semi_size):
    fromspace_(GetStartingAddress(), semi_size),
    tospace_(GetStartingAddress() + semi_size, semi_size),
    size_(size),
    semisize_(semi_size),
    table_(GetNumberOfNewPages()) {
   }

   virtual void SwapSpaces(){
     std::swap(fromspace_, tospace_);
   }

   inline uword GetPageAddressAt(const int64_t& index) const {
     if(index < 0 || index > GetNumberOfNewPages())
       return 0;
     return GetStartingAddress() + (index * GetNewPageSize());
   }

   inline int64_t GetPageIndex(Page* page) const {
     return static_cast<int64_t>((page->GetStartingAddress() - GetStartingAddress())) / GetNewPageSize();
   }
  public:
   NewZone() = delete;
   NewZone(const NewZone& rhs) = delete;
   ~NewZone() override = default;

   uword GetZoneStartingAddress() const {
     return (uword)this;
   }

   uword GetStartingAddress() const override {
     return GetZoneStartingAddress() + GetHeaderSize();
   }

   int64_t GetSize() const override {
     return size_;
   }

   int64_t GetTotalSize() const {
     return GetSize() + GetHeaderSize();
   }

   Semispace fromspace() const {
     return fromspace_;
   }

   Semispace& fromspace() {
     return fromspace_;
   }

   Semispace tospace() const {
     return tospace_;
   }

   Semispace& tospace() {
     return tospace_;
   }

   int64_t semisize() const{
     return semisize_;
   }

   virtual NewPage* GetPageAt(const int64_t& index) const {
     if(index < 0 || index > GetNumberOfNewPages())
       return nullptr;
     return new ((void*) GetPageAddressAt(index))NewPage();
   }

   virtual bool IsPageMarked(const int64_t index) const {
     return table_.Test(index);
   }

   inline bool IsPageMarked(NewPage* page) const {
     return IsPageMarked(GetPageIndex(page));
   }

   virtual uword TryAllocate(ObjectSize size);

   bool VisitPages(PageVisitor* vis) {
     return IteratePages<NewZone, NewZonePageIterator, PageVisitor>(this, vis);
   }

   bool VisitPages(NewPageVisitor* vis) {
     return IteratePages<NewZone, NewZonePageIterator, NewPageVisitor>(this, vis);
   }

   bool VisitMarkedPages(PageVisitor* vis) {
     return IteratePages<NewZone, NewZonePageIterator, PageVisitor>(this, vis);
   }

   bool VisitMarkedPages(NewPageVisitor* vis) {
     return IterateMarkedPages<NewZone, NewZonePageIterator, NewPageVisitor>(this, vis);
   }

   bool VisitPointers(RawObjectVisitor* vis) override {
     return IteratePointers<NewZone, NewZoneIterator, RawObjectVisitor>(this, vis);
   }

   bool VisitMarkedPointers(RawObjectVisitor* vis) override {
     return IterateMarkedPointers<NewZone, NewZoneIterator, RawObjectVisitor>(this, vis);
   }

   virtual bool MarkAllIntersectedBy(const Region& region);

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

   friend std::ostream& operator<<(std::ostream& stream, const NewZone& val){
     stream << "NewZone(";
     stream << "start=" << val.GetStartingAddressPointer() << ", ";
     stream << "size=" << Bytes(val.GetSize()) << ", ";
     stream << "fromspace=" << val.fromspace() << ", ";
     stream << "tospace=" << val.tospace()  << ", ";
     stream << "semi-size=" << Bytes(val.semisize());
     stream << ")";
     return stream;
   }
  public:
   static NewZone* New(const MemoryRegion& region);
 };
}

#endif//POSEIDON_HEAP_NEW_ZONE_H