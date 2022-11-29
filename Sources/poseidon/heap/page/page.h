#ifndef POSEIDON_HEAP_PAGE_H
#define POSEIDON_HEAP_PAGE_H

#include <ostream>

#include "poseidon/heap/section.h"
#include "poseidon/relaxed_atomic.h"
#include "poseidon/heap/page/page_tag.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon {
 class Page;
 class PageVisitor {
  protected:
   PageVisitor() = default;
  public:
   virtual ~PageVisitor() = default;
   virtual bool Visit(Page* page) = 0;
 };

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
   Page(const Page& rhs) = default;
   ~Page() override = default;

   uword GetStartingAddress() const override {
     return start_;
   }

   PageIndex GetIndex() const {
     return index_;
   }

   word GetSize() const override {
     return size_;
   }

   bool VisitPointers(RawObjectVisitor* vis) override {
     return IteratePointers<Page, PageIterator>(vis);
   }

   bool VisitPointers(const std::function<bool(Pointer*)>& vis) override {
     return IteratePointers<Page, PageIterator>(vis);
   }

   bool VisitMarkedPointers(RawObjectVisitor* vis) override {
     return IterateMarkedPointers<Page, PageIterator>(vis);
   }

   bool VisitMarkedPointers(const std::function<bool(Pointer*)>& vis) override {
     return IterateMarkedPointers<Page, PageIterator>(vis);
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
     stream << "size=" << value.GetSize();
     stream << ")";
     return stream;
   }
 };
}

#endif//POSEIDON_HEAP_PAGE_H