#ifndef POSEIDON_HEAP_PAGE_H
#define POSEIDON_HEAP_PAGE_H

#include <ostream>

#include "poseidon/heap/section.h"
#include "poseidon/relaxed_atomic.h"
#include "poseidon/platform/memory_region.h"

namespace poseidon{
 class PageVisitor {
  protected:
   PageVisitor() = default;
  public:
   virtual ~PageVisitor() = default;
 };

 class Page : public Section {
   friend class NewZone;
   friend class OldZone;

   friend class RawObject;//TODO: remove
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

     inline RawObject* current_ptr() const {
       return (RawObject*)current_address();
     }
    public:
     explicit PageIterator(T* page):
       page_(page),
       current_address_(page->GetStartingAddress()) {
     }
     ~PageIterator() override = default;

     RawObject* Next() override {
       auto next = current_ptr();
       current_address_ += next->GetTotalSize();
       return next;
     }
   };
  protected:
   Page() = default;
  public:
   ~Page() override = default;

   uword GetStartingAddress() const override {
     return (uword)this;
   }

   virtual void Reset() { //TODO: refactor
     memset(GetStartingAddressPointer(), 0, GetSize());
   }
 };
}

#endif//POSEIDON_HEAP_PAGE_H