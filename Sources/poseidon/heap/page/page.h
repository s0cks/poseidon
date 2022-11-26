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

   friend class Pointer;//TODO: remove
   friend class NewPageTest; //TODO: remove
   friend class OldPageTest; //TODO: remove
  protected:
   template<class T>
   class PageIterator : public RawObjectPointerIterator {
    protected:
     T* page_;
     uword current_address_;

     inline T* page() const {
       return page_;
     }

     inline uword current_address() const {
       return current_address_;
     }

     inline Pointer* current_ptr() const {
       return (Pointer*)current_address();
     }
    public:
     explicit PageIterator(T* page):
       page_(page),
       current_address_(page->GetStartingAddress()) {
     }
     ~PageIterator() override = default;

     Pointer* Next() override {
       auto next = current_ptr();
       current_address_ += next->GetTotalSize();
       return next;
     }
   };
  protected:
   RelaxedAtomic<RawPageTag> tag_;
   uword start_;

   Page():
    Section(),
    tag_(0),
    start_(0) {
   }

   Page(const RawPageTag tag, const uword start):
    Section(),
    tag_(tag),
    start_(start) {
   }

   inline RawPageTag raw_tag() const {
     return (RawPageTag)tag_;
   }

   inline void set_raw_tag(const RawPageTag& value) {
     tag_ = value;
   }
  public:
   Page(const Page& rhs) = default;
   ~Page() override = default;

   uword GetStartingAddress() const override {
     return start_;
   }

   PageIndex GetIndex() const {
     return PageTag::Index::Decode(raw_tag());
   }

   int64_t GetSize() const override {
     return PageTag::Size::Decode(raw_tag());
   }

   Page& operator=(const Page& rhs) {
     if(&rhs == this)
       return *this;
     Section::operator=(rhs);
     tag_ = rhs.raw_tag();
     start_ = rhs.GetStartingAddress();
     return *this;
   }
 };
}

#endif//POSEIDON_HEAP_PAGE_H