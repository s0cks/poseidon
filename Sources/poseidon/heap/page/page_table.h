#ifndef POSEIDON_PAGE_TABLE_H
#define POSEIDON_PAGE_TABLE_H

#include "poseidon/bitset.h"
#include "poseidon/region.h"

namespace poseidon {
 class PageTable : public Region {
  protected:
   BitSet marked_;
   uword start_;
   int64_t size_;
   int64_t page_size_;
  public:
   PageTable() = default;
   ~PageTable() override = default;

   uword GetStartingAddress() const override {
     return start_;
   }

   int64_t GetPageSize() const {
     return page_size_;
   }

   int64_t GetSize() const override {
     return size_;
   }

   BitSet marked() const {
     return marked_;
   }
 };
}

#endif // POSEIDON_PAGE_TABLE_H