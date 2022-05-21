#ifndef POSEIDON_OLD_PAGE_H
#define POSEIDON_OLD_PAGE_H

#include "poseidon/utils.h"
#include "poseidon/bitset.h"
#include "poseidon/common.h"
#include "poseidon/heap/section.h"

namespace poseidon{
 class OldPage : public Section{
   friend class OldPageTable;
  private:
   int64_t index_;

   OldPage(int64_t index, uword start, int64_t size):
     Section(start, size),
     index_(index){
   }
  public:
   OldPage():
     Section(),
     index_(0){
   }
   OldPage(const OldPage& rhs):
     Section(rhs),
     index_(rhs.index()){
   }
   ~OldPage() override = default;

   int64_t index() const{
     return index_;
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
}

#endif//POSEIDON_OLD_PAGE_H