#ifndef POSEIDON_PAGE_TABLE_H
#define POSEIDON_PAGE_TABLE_H

#include "poseidon/bitset.h"
#include "poseidon/heap/section.h"
#include "poseidon/heap/page/page.h"

namespace poseidon {
 class PageTable {
  protected:
   BitSet marked_;
   Page* pages_;
   word num_pages_;

   inline void Resize(const word num_pages) {
     if(num_pages <= 0 || num_pages <= GetNumberOfPages())
       return;
     auto new_size = static_cast<word>(RoundUpPowTwo(num_pages));
     auto new_pages = new Page[new_size];
     std::copy_n(pages_, std::min(num_pages_, new_size), new_pages);
     pages_ = new_pages;
     num_pages_ = new_size;
   }
  public:
   PageTable():
    marked_(),
    pages_(nullptr),
    num_pages_(0) {
   }
   PageTable(uword start, word size, word page_size):
    marked_(size / page_size),
    pages_(new Page[1]),
    num_pages_(1) {
     pages_[0] = Page(0, start, size);
   }
   PageTable(const PageTable& rhs):
    marked_(rhs.marked_),
    pages_(new Page[rhs.num_pages_]),
    num_pages_(rhs.num_pages_) {
     std::copy_n(rhs.pages_, rhs.num_pages_, pages_);
   }
   ~PageTable() {
     if(pages_)
       free(pages_);
   }

   Page* pages() {
     return pages_;
   }

   Page* pages(const word index) {
     if(index < 0 || index > GetNumberOfPages())
       return nullptr;
     return pages() + index;
   }

   Page* begin() {
     return pages();
   }

   Page* end() {
     return pages() + GetNumberOfPages();
   }

   word GetNumberOfPages() const {
     return num_pages_;
   }

   bool IsMarked(const word index) {
     return marked_.Test(index);
   }

   void Mark(const word index) {
     marked_.Set(static_cast<int64_t>(index), true);
   }

   void Unmark(const word index) {
     marked_.Set(static_cast<int64_t>(index), false);
   }

   void MarkAllIntersectedBy(Region* region);

   explicit operator BitSet() const {
     return marked_;
   }

   Page& operator[](const word index) {
     return pages_[index];
   }

   Page operator[](const word index) const {
     return pages_[index];
   }

   PageTable& operator=(const PageTable& rhs) {
     if(&rhs == this)
       return *this;
     marked_ = rhs.marked_;
     if(pages_) {
       free(pages_);
       pages_ = nullptr;
       num_pages_ = 0;
     }

     if(rhs.pages_) {
       pages_ = new Page[rhs.num_pages_];
       std::copy_n(rhs.pages_, rhs.num_pages_, pages_);
       num_pages_ = rhs.num_pages_;
     }
     return *this;
   }

   friend std::ostream& operator<<(std::ostream& stream, const PageTable& table) {
     return stream << (const BitSet)table;
   }
 };
}

#endif // POSEIDON_PAGE_TABLE_H