#ifndef POSEIDON_PAGE_TABLE_H
#define POSEIDON_PAGE_TABLE_H

#include "poseidon/bitset.h"
#include "poseidon/heap/page.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon {
 class PageTable {
   friend class Zone;
   friend class NewZone;
  public:
   class PageTableIterator {
    protected:
     const PageTable* table_;
     PageIndex current_;
     PageIndex size_;

     inline PageIndex current_index() const {
       return current_;
     }

     inline PageIndex length() const {
       return size_;
     }

     inline Page* pages(const PageIndex index) const {
       return table_->pages(index);
     }
    public:
     explicit PageTableIterator(const PageTable* table):
      table_(table),
      current_(0),
      size_(table->size()){
     }
     ~PageTableIterator() = default;

     bool HasNext() const {
       return current_index() < length();
     }

     Page* Next() {
       auto next = pages(current_index());
       current_ += 1;
       return next;
     }
   };
  protected:
   Page** pages_;
   PageIndex size_;
   BitSet table_;
  public:
   explicit PageTable(const PageIndex num_pages):
    pages_(nullptr),
    size_(num_pages),
    table_(num_pages) {
     if (num_pages > 0) {
       LOG(INFO) << "allocating " << num_pages << " pages.";
       pages_ = new Page*[num_pages];
     }
   }
   PageTable(const PageTable& rhs) = default;
   virtual ~PageTable() = default;

   PageIndex size() const {
     return size_;
   }

   Page* pages(const PageIndex index) const {
     if(index < 0 || index > size())
       return nullptr;
     return pages_[index];
   }

   Page** pages() const {
     return pages_;
   }

   Page** pages_begin() const {
     return pages();
   }

   Page** pages_end() const {
     return pages() + size();
   }

   void Mark(Page* page) {
     table_.Set(page->index(), true);
     page->SetMarkedBit(true);
   }

   void MarkAll(const Region& region);

   inline void
   MarkAll(const RawObject* raw) {
     return MarkAll((*raw));
   }

   void Unmark(Page* page) {
     table_.Set(page->index(), false);
     page->SetMarkedBit(false);
   }

   inline bool IsMarked(Page* page) const {
     return table_.Test(page->index()) && page->marked();
   }

   void VisitPages(PageVisitor* vis) const;
   void VisitMarkedPages(PageVisitor* vis) const;
   void VisitUnmarkedPages(PageVisitor* vis) const;

   PageTable& operator=(const PageTable& rhs) = default;

   Page* operator[](const PageIndex index) const {
     return pages(index);
   }

   friend std::ostream& operator<<(std::ostream& stream, const PageTable& val) {
     stream << "PageTable(";
     stream << "size=" << val.size();
     return stream << ")";
   }
 };
}

#endif // POSEIDON_PAGE_TABLE_H