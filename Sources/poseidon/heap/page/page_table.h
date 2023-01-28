#ifndef POSEIDON_PAGE_TABLE_H
#define POSEIDON_PAGE_TABLE_H

#include "poseidon/array.h"
#include "poseidon/bitset.h"
#include "poseidon/heap/section.h"
#include "poseidon/heap/page/page.h"

namespace poseidon {
 class PageTable : public Section {
   friend class PageTablePageIterator;
  public:
   class PageTablePageIterator {
    protected:
     const PageTable* table_;
     PageIndex current_;

     inline const PageTable* table() const {
       return table_;
     }

     inline PageIndex current() const {
       return current_;
     }

     inline PageIndex num_pages() const {
       return table()->pages_.Length();
     }

     inline Page* current_page() const {
       return table()->pages_[current()];
     }
    public:
     explicit PageTablePageIterator(const PageTable* table):
      table_(table),
      current_(0) {
     }
     virtual ~PageTablePageIterator() = default;

     bool HasNext() const {
       return current() < num_pages() &&
              current_page() != nullptr;
     }

     Page* Next() {
       auto next = current_page();
       current_ += 1;
       return next;
     }
   };

   class PageTablePointerIterator : public RawObjectPointerIterator { //TODO: implement PageTableIterator
    protected:
     const PageTable* table_;

     inline const PageTable* table() const {
       return table_;
     }
    public:
     explicit PageTablePointerIterator(const PageTable* table):
      RawObjectPointerIterator(),
      table_(table) {
     }
     ~PageTablePointerIterator() override = default;

     bool HasNext() const override {
       return false;
     }

     Pointer* Next() override {
       return nullptr;
     }
   };
  protected:
   BitSet marked_;
   RegionSize page_size_;
   Array<Page*> pages_;

   inline void
   InitializePageTable(const Region& region, const RegionSize page_size) {
     auto num_pages = region.GetSize() / page_size;
     DLOG(INFO) << "initializing PageTable for " << region << " with " << num_pages << " " << Bytes(page_size) << " pages";
     for(auto idx = 0; idx < num_pages; idx++)
       pages_.Add(new Page(idx, region.GetStartingAddress() + (idx * page_size), page_size));
   }

   inline word
   GetPageOffset(Page* page) const {
     return static_cast<word>(page->GetStartingAddress() - GetStartingAddress());
   }

   inline PageIndex
   GetPageIndex(Page* page) const {
     auto offset = GetPageOffset(page);
     return offset / GetPageSize();
   }
  public:
   PageTable() = delete;
   PageTable(const Region& region, const RegionSize page_size):
    Section(region),
    marked_(region.GetSize() / page_size),
    page_size_(page_size),
    pages_() {
     InitializePageTable(region, page_size);
   }
   PageTable(const PageTable& rhs) = delete;
   ~PageTable() override = default;

   RegionSize GetPageSize() const {
     return page_size_;
   }

   PageIndex GetNumberOfPages() const {
     return pages_.Length();
   }

   bool IsMarked(const PageIndex index) const {
     return marked_.Test(index);
   }

   void Mark(const PageIndex index) {
     marked_.Set(index, true);
   }

   void Unmark(const PageIndex index) {
     marked_.Set(index, false);
   }

   void MarkAllIntersectedBy(const Region& region);

   bool VisitPages(PageVisitor* vis);

   inline bool VisitPages(const std::function<bool(Page*)>& func) {
     auto vis = PageVisitorWrapper(func);
     return VisitPages(&vis);
   }

   bool VisitMarkedPages(PageVisitor* vis);

   inline bool VisitMarkedPages(const std::function<bool(Page*)>& func) {
     auto vis = PageVisitorWrapper(func);
     return VisitMarkedPages(&vis);
   }

   bool VisitPointers(RawObjectVisitor* vis) override;
   bool VisitMarkedPointers(RawObjectVisitor* vis) override;

   bool VisitNewPointers(RawObjectVisitor* vis) override {
     NOT_IMPLEMENTED(ERROR); //TODO: implement
     return false;
   }

   bool VisitOldPointers(RawObjectVisitor* vis) override {
     NOT_IMPLEMENTED(ERROR); //TODO: implement
     return false;
   }

   void ClearTable() {
     NOT_IMPLEMENTED(FATAL); //TODO: implement
   }

   explicit operator BitSet() const {
     return marked_;
   }

   PageTable& operator=(const PageTable& rhs) = delete;

   friend std::ostream& operator<<(std::ostream& stream, const PageTable& table) {
     stream << "PageTable(";
     stream << "start=" << table.GetStartingAddressPointer() << ", ";
     stream << "size=" << Bytes(table.GetSize()) << ", ";
     stream << "end=" << table.GetEndingAddressPointer() << ", ";
     stream << "page_size=" << Bytes(table.GetPageSize()) << ", ";
     stream << "num_pages=" << table.GetNumberOfPages() << ", ";
     stream << "marked=" << table.marked_;
     stream << ")";
     return stream;
   }
 };

 template<const google::LogSeverity Severity = google::INFO>
 class PageTablePrinter : public PageVisitor {
  protected:
   const PageTable* table_;
   std::string header_text_;
   int header_size_;

   std::string GetHeader() {
     auto remaining = header_size_ - (header_text_.length() + 2);
     std::stringstream ss;
     ss << std::string(remaining / 2, '=');
     ss << ' ';
     ss << header_text_;
     ss << ' ';
     ss << std::string(remaining / 2, '=');
     return ss.str();
   }

   std::string GetFooter() {
     return std::string(header_size_, '=');
   }
  public:
   explicit PageTablePrinter(const PageTable* table, int header_size, std::string header_text):
    PageVisitor(),
    table_(table),
    header_size_(header_size),
    header_text_(std::move(header_text)) {
   }
   ~PageTablePrinter() override = default;

   inline const PageTable* table() const {
     return table_;
   }

   void VisitStart() {
     LOG_AT_LEVEL(Severity) << GetHeader();
     LOG_AT_LEVEL(Severity) << *table();
   }

   bool Visit(Page* page) override {
     LOG_AT_LEVEL(Severity) << " - " << (*page);
     return true;
   }

   void VisitEnd() {
     LOG_AT_LEVEL(Severity) << GetFooter();
   }

   static inline void
   Print(PageTable* table, int header_size, std::string header_text) {
     PageTablePrinter printer(table, header_size, std::move(header_text));
     printer.VisitStart();
     LOG_IF(ERROR, !table->VisitPages(&printer)) << "failed to print pages";
     printer.VisitEnd();
   }

   static inline void
   Print(PageTable& table, int header_size, std::string header_text) {
     return Print(&table, header_size, std::move(header_text));
   }
 };
}

#endif // POSEIDON_PAGE_TABLE_H