#ifndef POSEIDON_HEAP_PAGE_H
#define POSEIDON_HEAP_PAGE_H

#include <ostream>

#include "poseidon/heap/section.h"
#include "poseidon/relaxed_atomic.h"
#include "page_tag.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon {
 class Page;
 class PageVisitor : public Visitor<Page> {
  protected:
   PageVisitor() = default;
  public:
   ~PageVisitor() override = default;
 };
 DEFINE_VISITOR_WRAPPER(PageVisitor, Page);

 class Page : public Section {
   friend class NewZone;
   friend class OldZone;
  protected:
   word index_;

   class PageIterator : public RawObjectPointerIterator {
    protected:
     const Page* page_;
     uword current_;

     inline const Page*
     page() const {
       return page_;
     }

     inline uword
     current_address() const {
       return current_;
     }

     inline Pointer*
     current_ptr() const {
       return (Pointer*)current_address();
     }
    public:
     explicit PageIterator(const Page* page):
      RawObjectPointerIterator(),
      page_(page),
      current_(page->GetStartingAddress()) {
     }
     ~PageIterator() override = default;

     bool HasNext() const override {
       return current_address() >= page()->GetStartingAddress() &&
              current_address() <= page()->GetEndingAddress() &&
              !current_ptr()->IsFree() &&
              current_ptr()->GetSize() > 0;
     }

     Pointer* Next() override {
       auto next = current_ptr();
       current_ += next->GetTotalSize();
       return next;
     }
   };
  public:
   Page() = default;
   Page(const word index, const uword start, const word size):
    Section(start, size),
    index_(index) {
   }
   Page(const PageIndex index, const Region& region):
    Section(region),
    index_(index) {
   }
   Page(const Page& rhs) = default;
   ~Page() override = default;

   PageIndex GetIndex() const {
     return index_;
   }

   bool VisitPointers(RawObjectVisitor* vis) const override {
     return IteratePointers<Page, PageIterator>(vis);
   }

   bool VisitMarkedPointers(RawObjectVisitor* vis) const override {
     return IterateMarkedPointers<Page, PageIterator>(vis);
   }

   bool VisitUnmarkedPointers(RawObjectVisitor* vis) const override {
     return IterateUnmarkedPointers<Page, PageIterator>(vis);
   }

   bool VisitNewPointers(RawObjectVisitor* vis) const override {
     return IterateNewPointers<Page, PageIterator>(vis);
   }

   bool VisitOldPointers(RawObjectVisitor* vis) const override {
     return IterateOldPointers<Page, PageIterator>(vis);
   }

   Page& operator=(const Page& rhs) {
     if(&rhs == this)
       return *this;
     Section::operator=(rhs);
     index_ = rhs.index_;
     return *this;
   }

   friend std::ostream& operator<<(std::ostream& stream, const Page& value) {
     stream << "Page(";
     stream << "index=" << value.GetIndex() << ", ";
     stream << "start=" << value.GetStartingAddressPointer() << ", ";
     stream << "size=" << Bytes(value.GetSize());
     stream << ")";
     return stream;
   }
 };
}

#endif//POSEIDON_HEAP_PAGE_H