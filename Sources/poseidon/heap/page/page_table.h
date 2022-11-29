#ifndef POSEIDON_PAGE_TABLE_H
#define POSEIDON_PAGE_TABLE_H

#include "poseidon/bitset.h"
#include "poseidon/region.h"

namespace poseidon {
 class PageTable : public Region {
  protected:
   BitSet marked_;
   uword start_;
   word size_;
   word page_size_;
  public:
   PageTable(uword start, word size, word page_size):
    marked_(size / page_size),
    start_(start),
    size_(size),
    page_size_(page_size) {
   }
   ~PageTable() override = default;

   uword GetStartingAddress() const override {
     return start_;
   }

   word GetPageSize() const {
     return page_size_;
   }

   word GetSize() const override {
     return size_;
   }

   word GetNumberOfPages() const {
     return size_ / page_size_;
   }

   BitSet marked() const {
     return marked_;
   }

   void MarkAllIntersectedBy(Region* region);

   friend std::ostream& operator<<(std::ostream& stream, const PageTable& table) {
     return stream << table.marked();
   }
 };
}

#endif // POSEIDON_PAGE_TABLE_H