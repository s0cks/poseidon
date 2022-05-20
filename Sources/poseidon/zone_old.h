#ifndef POSEIDON_ZONE_OLD_H
#define POSEIDON_ZONE_OLD_H

#include "poseidon/zone.h"

namespace poseidon{
 class OldPage{
   friend class OldPageTable;
  private:
   int64_t index_;
   uword start_;
   int64_t size_;

   OldPage(int64_t index, uword start, int64_t size):
    index_(index),
    start_(start),
    size_(size){
   }
  public:
   OldPage() = default;
   OldPage(const OldPage& rhs):
    index_(rhs.index()),
    start_(rhs.starting_address()),
    size_(rhs.size()){
   }
   ~OldPage() = default;

   int64_t index() const{
     return index_;
   }

   uword starting_address() const{
     return start_;
   }

   int64_t size() const{
     return size_;
   }

   uword ending_address() const{
     return starting_address() + size();
   }

   bool Contains(uword address) const{
     return starting_address() <= address
         && ending_address() >= address;
   }
 };

 class OldPageTable{
   friend class OldZone;
  private:
   OldPage* pages_;
   int64_t num_pages_;
   BitSet marked_;

   void CreatePagesForRange(uword start, int64_t sz, int64_t page_size){
     PSDN_ASSERT(IsPow2(sz));
     PSDN_ASSERT(IsPow2(page_size));
     PSDN_ASSERT((sz % page_size) == 0);
     PSDN_ASSERT((sz / page_size) == size());

     int64_t index = 0;
     for(auto current = start; current < (start + sz); current += page_size, index++){
       pages_[index] = OldPage(index, current, page_size);
     }
   }
  public:
   OldPageTable():
    pages_(nullptr),
    num_pages_(0),
    marked_(){
   }
   explicit OldPageTable(int64_t num_pages);
   OldPageTable(const OldPageTable& rhs);
   ~OldPageTable(){
     delete[] pages_;
   }

   int64_t size() const{
     return num_pages_;
   }

   bool IsMarked(int64_t idx) const{
     PSDN_ASSERT(idx >= 0);
     PSDN_ASSERT(idx <= size());
     return marked_.Test(idx);
   }

   bool IsMarked(OldPage* page) const{
     PSDN_ASSERT(page != nullptr);
     return IsMarked(page->index());
   }

   void Mark(int64_t idx){
     PSDN_ASSERT(idx >= 0);
     PSDN_ASSERT(idx <= size());
     return marked_.Set(idx, true);
   }

   void Mark(OldPage* page){
     PSDN_ASSERT(page != nullptr);
     return Mark(page->index());
   }

   void Unmark(int64_t idx){
     PSDN_ASSERT(idx >= 0);
     PSDN_ASSERT(idx <= size());
     return marked_.Set(idx, false);
   }

   void Unmark(OldPage* page){
     PSDN_ASSERT(page != nullptr);
     return Unmark(page->index());
   }

   OldPage* page(int64_t idx) const{
     PSDN_ASSERT(idx >= 0);
     PSDN_ASSERT(idx <= size());
     return &pages_[idx];
   }

   OldPage* begin() const{
     return &pages_[0];
   }

   OldPage* pages(int64_t idx) const{
     PSDN_ASSERT(idx >= 0);
     PSDN_ASSERT(idx <= size());
     return &pages_[idx];
   }

   OldPage* end() const{
     return &pages_[size()];
   }

   void VisitAll(std::function<bool(OldPage*)>& vis) const{
     for(auto& page : *this){
       if(!vis(&page))
         return;
     }
   }

   void VisitMarked(std::function<bool(OldPage*)>& vis) const{
     for(auto& page : *this){
       if(!vis(&page))
         return;
     }
   }

   OldPage& operator[](int64_t idx) const{
     return pages_[idx];
   }

   friend std::ostream& operator<<(std::ostream& stream, const OldPageTable& val){
     return stream << "OldPageTable(num_pages=" << val.size() << ", marked=" << val.marked_ << ")";
   }
 };

 class OldZone : public Zone{
   friend class OldZoneTest;

   friend class SerialSweeper;
   friend class ParallelSweeper;
  protected:
   FreeList free_list_;
   OldPageTable pages_;

   static inline int64_t
   GetNumberOfPages(int64_t size, int64_t page_size){
     PSDN_ASSERT(IsPow2(size));
     PSDN_ASSERT(IsPow2(page_size));
     PSDN_ASSERT(size % page_size == 0);
     return size / page_size;
   }

   static inline int64_t
   CalculateTableSize(int64_t size, int64_t page_size){
     return size / page_size;
   }
  public:
   explicit OldZone(uword start, int64_t size, int64_t page_size):
     Zone(start, size),
     pages_(CalculateTableSize(size, page_size)),
     free_list_(start, size){
     pages_.CreatePagesForRange(start, size, page_size);
   }
   explicit OldZone(MemoryRegion* region, int64_t offset, int64_t size, int64_t page_size):
     OldZone(region->GetStartingAddress() + offset, size, page_size){
   }
   ~OldZone() override = default;

   FreeList* free_list(){//TODO: visible for testing
     return &free_list_;
   }

   uword TryAllocate(int64_t size) override;

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

#endif//POSEIDON_ZONE_OLD_H