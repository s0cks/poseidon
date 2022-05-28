#ifndef POSEIDON_OLD_PAGE_H
#define POSEIDON_OLD_PAGE_H

#include "poseidon/utils.h"
#include "poseidon/bitset.h"
#include "poseidon/common.h"
#include "poseidon/heap/section.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon{
 class OldZone;
 class OldPageTable;
 class OldPage : public AllocationSection{
   friend class OldPageTest;
   friend class OldPageTable;
  protected:
   OldPageTable* table_;
   int64_t index_;

   OldPage(OldPageTable* table, int64_t index, uword start, int64_t size):
     AllocationSection(start, size),
     table_(table),
     index_(index){
   }

   OldPage(OldPageTable* table, int64_t index, MemoryRegion* region, int64_t offset, int64_t size):
     OldPage(table, index, region->GetStartingAddress() + offset, size){
   }

   OldPage(OldPageTable* table, int64_t index, MemoryRegion* region, int64_t size):
     OldPage(table, index, region, 0, size){
   }

   OldPage(OldPageTable* table, int64_t index, MemoryRegion* region):
     OldPage(table, index, region, region->size()){
   }

   explicit OldPage(OldPageTable* table, MemoryRegion* region):
     OldPage(table, 0, region){
   }

   inline OldPageTable* GetTable() const{
     return table_;
   }
  public:
   OldPage():
     AllocationSection(),
     table_(nullptr),
     index_(0){
   }
   OldPage(const OldPage& rhs):
     AllocationSection(rhs),
     table_(rhs.GetTable()),
     index_(rhs.GetIndex()){
   }
   ~OldPage() override = default;

   int64_t GetIndex() const{
     return index_;
   }

   uword TryAllocate(int64_t size) override{
     return RawObject::TryAllocateOldIn(this, size);
   }

   OldPage& operator=(const OldPage& rhs) = default;

   friend std::ostream& operator<<(std::ostream& stream, const OldPage& val){
     stream << "OldPage(";
     stream << "index=" << val.GetIndex() << ", ";
     stream << "start=" << val.GetStartingAddress() << ", ";
     stream << "size=" << Bytes(val.GetSize());
     stream << ")";
     return stream;
   }

   friend bool operator==(const OldPage& lhs, const OldPage& rhs){
     return lhs.GetIndex() == rhs.GetIndex()
         && lhs.GetStartingAddress() == rhs.GetStartingAddress()
         && lhs.GetSize() == rhs.GetSize();
   }

   friend bool operator!=(const OldPage& lhs, const OldPage& rhs){
     return !operator==(lhs, rhs);
   }

   friend bool operator<(const OldPage& lhs, const OldPage& rhs){
     return lhs.GetIndex() < rhs.GetIndex();
   }
 };

 class OldPageTable{
   friend class OldZone;
  private:
   OldZone* zone_;
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
       pages_[index] = OldPage(this, index, current, page_size);
     }
   }
  public:
   OldPageTable():
     zone_(nullptr),
     pages_(nullptr),
     num_pages_(0),
     marked_(){
   }
   OldPageTable(uword start, int64_t sz, int64_t page_size):
     pages_(nullptr),
     num_pages_(0),
     marked_(){
     PSDN_ASSERT(start > 0);
     PSDN_ASSERT(sz > 0);
     PSDN_ASSERT(IsPow2(sz));
     PSDN_ASSERT(page_size > 0);
     PSDN_ASSERT(IsPow2(page_size));
     PSDN_ASSERT((sz % page_size) == 0);
     num_pages_ = (sz / page_size);
     marked_ = BitSet(num_pages_);
     pages_ = new OldPage[num_pages_];

     auto current_address = start;
     for(auto idx = 0; idx < num_pages_; idx++, current_address += page_size){
       pages_[idx] = OldPage(this, idx, current_address, page_size);
     }
   }
   OldPageTable(const OldPageTable& rhs):
     zone_(rhs.zone_),
     pages_(new OldPage[rhs.size()]),
     num_pages_(rhs.size()),
     marked_(rhs.marked_){
     std::copy(begin(), end(), rhs.begin());
   }
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
     return IsMarked(page->GetIndex());
   }

   void Mark(int64_t idx){
     DLOG(INFO) << "marking #" << idx;
     PSDN_ASSERT(idx >= 0);
     PSDN_ASSERT(idx <= size());
     return marked_.Set(idx, true);
   }

   void Mark(OldPage* page){
     PSDN_ASSERT(page != nullptr);
     return Mark(page->GetIndex());
   }

   void Unmark(int64_t idx){
     PSDN_ASSERT(idx >= 0);
     PSDN_ASSERT(idx <= size());
     return marked_.Set(idx, false);
   }

   void Unmark(OldPage* page){
     PSDN_ASSERT(page != nullptr);
     return Unmark(page->GetIndex());
   }

   OldPage* page(int64_t idx) const{
     PSDN_ASSERT(idx >= 0);
     PSDN_ASSERT(idx <= size());
     return &pages_[idx];
   }

   OldPage* begin() const{
     return pages(0);
   }

   OldPage* pages(int64_t idx) const{
     PSDN_ASSERT(idx >= 0);
     PSDN_ASSERT(idx <= size());
     return &pages_[idx];
   }

   OldPage* end() const{
     return pages(size());
   }

   BitSet& marked(){
     return marked_;
   }

   void VisitAll(std::function<bool(OldPage * )>& vis) const{
     for(auto& page : *this){
       if(!vis(&page))
         return;
     }
   }

   void VisitMarked(std::function<bool(OldPage * )>& vis) const{
     for(auto& page : *this){
       if(!vis(&page))
         return;
     }
   }

   OldPage* operator[](int64_t idx) const{
     return &pages_[idx];
   }

   friend std::ostream& operator<<(std::ostream& stream, const OldPageTable& val){
     return stream << "OldPageTable(num_pages=" << val.size() << ", marked=" << val.marked_ << ")";
   }
 };
}

#endif//POSEIDON_OLD_PAGE_H