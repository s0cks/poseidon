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
   uword current_;
   uword fromspace_;
   uword tospace_;
   int64_t size_;
   int64_t semisize_; //TODO: remove
   BitSet table_;

   NewZone(const int64_t& size, const int64_t& semi_size):
    current_(GetStartingAddress()),
    fromspace_(GetStartingAddress()),
    tospace_(GetStartingAddress() + semi_size),
    size_(size),
    semisize_(semi_size),
    table_(GetNumberOfNewPages()) {
   }

   virtual void SwapSpaces(){
     std::swap(fromspace_, tospace_);
     current_ = tospace_;
   }

   bool InitializePages(const MemoryRegion& region);
   uword TryAllocate(ObjectSize size);

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

   inline uword GetPageAddressAt(const int64_t& index) const {
     if(index < 0 || index > GetNumberOfNewPages())
       return 0;
     return GetStartingAddress() + (index * GetNewPageSize());
   }

   inline int64_t GetPageIndex(NewPage* page) const {
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

   virtual NewPage* GetPageAt(const int64_t& index) const {
     if(index < 0 || index > GetNumberOfNewPages())
       return nullptr;
     return new ((void*) GetPageAddressAt(index))NewPage();
   }

   virtual bool IsPageMarked(const int64_t& index) const {
     return table_.Test(index);
   }

   bool VisitPages(NewPageVisitor* vis);
   bool VisitMarkedPages(NewPageVisitor* vis);
   bool VisitPointers(RawObjectVisitor* vis) override;
   bool VisitMarkedPointers(RawObjectVisitor* vis) override;

   virtual void MarkAllIntersectedBy(const Region& region);

   virtual inline void MarkPage(const int64_t& index) {
     return table_.Set(index, true);
   }

   virtual inline void MarkPage(NewPage* page) {
     return MarkPage(GetPageIndex(page));
   }

   virtual inline void UnmarkPage(const int64_t& index) {
     return table_.Set(index, false);
   }

   virtual inline void UnmarkPage(NewPage* page) {
     return UnmarkPage(GetPageIndex(page));
   }

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
  public:
   static NewZone* New(const MemoryRegion& region);
 };
}

#endif//POSEIDON_HEAP_NEW_ZONE_H